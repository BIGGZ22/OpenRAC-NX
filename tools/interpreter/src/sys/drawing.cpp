// SPDX-License-Identifier: GPL-3.0-or-later
// Copyright (c) 2026 the OpenRAC contributors

/**
 * The drawing path: gathering what the EE's side sends, the queue and the drawing thread.
 *
 * Every command is run in the order it was given. The EE's side waits for the drawing side only
 * where it must: `sync`, and `present` when two presents are already waiting.
 */

#include "drawing.h"

#include <utility>

namespace sys {

Drawing::~Drawing() {
    // Without a thread there is nothing to stop.
    if (thread_.joinable()) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            quit_ = true;
        }

        // Notify after the lock is gone, so the thread does not wake only to block on it.
        work_.notify_all();
        thread_.join();
    }

    // What the GIF was asked to do refers to this object.
    graphics_.gif.sync();
}

void Drawing::start() {
    // A second call would start a second thread.
    if (!thread_.joinable()) {
        // The GIF and the GS on one thread, VIF1 and VU1 on this one.
        graphics_.gif.start();
        thread_ = std::thread([this] { loop(); });
    }
}

void Drawing::gather(Kind kind, const u8* data, std::size_t bytes) {
    // Bytes for another unit cannot share a command: send what is open first, to keep the order.
    if (open_used_ && open_.kind != kind) {
        send();
    }

    // Start a new command, taking a spare buffer for its bytes if it has none.
    if (!open_used_) {
        open_.kind = kind;
        open_used_ = true;

        // The drawing thread puts emptied buffers in `spare_`, so the lock is needed.
        if (open_.data.capacity() == 0) {
            std::lock_guard<std::mutex> lock(mutex_);
            if (!spare_.empty()) {
                open_.data = std::move(spare_.back());
                spare_.pop_back();
            }
        }
    }

    open_.data.insert(open_.data.end(), data, data + bytes);
}

void Drawing::send() {
    // Nothing gathered since the last send.
    if (!open_used_) {
        return;
    }

    open_used_ = false;
    Command command = std::move(open_);
    open_ = Command{};
    give(std::move(command));
}

void Drawing::privileged(u32 address, u64 value) {
    // Earlier data goes first: the register write must come after it.
    send();

    Command command;
    command.kind = kPrivileged;
    command.address = address;
    command.value = value;
    give(std::move(command));
}

void Drawing::vblank(bool odd_field) {
    // Data given before the blank must be drawn before the blank starts.
    send();

    Command command;
    command.kind = kVblank;
    command.value = odd_field ? 1 : 0;
    give(std::move(command));
}

void Drawing::present() {
    // The picture must show everything given before it.
    send();

    // On the drawing thread at most two presents wait; a third makes the caller wait here.
    if (thread_.joinable()) {
        std::unique_lock<std::mutex> lock(mutex_);
        done_.wait(lock, [this] { return presents_waiting_ < 2; });
        presents_waiting_++;
    }

    Command command;
    command.kind = kPresent;
    give(std::move(command));
}

void Drawing::give(Command&& command) {
    // Without a drawing thread, the caller's thread does the work now.
    if (!thread_.joinable()) {
        run(command);
        return;
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push_back(std::move(command));
    }

    // Wake the drawing thread after the lock is gone, so it does not wake only to block on it.
    work_.notify_one();
}

void Drawing::sync() {
    // Gathered data counts as given.
    send();

    // Wait for the drawing thread to take every command and finish the last one.
    if (thread_.joinable()) {
        std::unique_lock<std::mutex> lock(mutex_);
        done_.wait(lock, [this] { return queue_.empty() && !busy_; });
    }

    // This thread has nothing to give the GIF now, so the caller may ask for it.
    graphics_.gif.sync();
    graphics_.gs.finish();
}

bool Drawing::picture(ps2::Image& out) {
    std::lock_guard<std::mutex> lock(picture_mutex_);

    // With the display off there is no picture to copy.
    if (picture_shown_) {
        out = picture_;
    }

    return picture_shown_;
}

void Drawing::run(Command& command) {
    switch (command.kind) {
        case kVif:
            // VIF1 decodes the command stream and passes packets on to the GIF itself.
            graphics_.vif.write(command.data.data(), command.data.size());
            break;

        case kGif:
            // The GIF takes whole quadwords of 16 bytes, here on path 3.
            graphics_.gif.write(3, command.data.data(), command.data.size() / 16);
            break;

        case kPrivileged:
            // Run on the GS's side, after the packets given before it.
            graphics_.gif.run([this, address = command.address, value = command.value] {
                graphics_.gs.write_privileged(address, value);
            });
            break;

        case kVblank:
            // The value holds 1 for an odd field.
            graphics_.gif.run([this, odd = command.value != 0] { graphics_.gs.vblank(odd); });
            break;

        case kPresent:
            // Take the picture on the GS's side, after everything given before it is drawn.
            graphics_.gif.run([this, counted = thread_.joinable()] {
                ps2::Image buffer;
                {
                    // The buffer is used again; only its contents are replaced.
                    std::lock_guard<std::mutex> lock(picture_mutex_);
                    buffer = std::move(spare_picture_);
                }

                // Called by the GS when the picture is taken, on the thread that draws.
                graphics_.gs.display_later(
                    std::move(buffer), [this, counted](bool shown, ps2::Image& image) {
                        {
                            std::lock_guard<std::mutex> lock(picture_mutex_);

                            // The new picture replaces the old, whose buffer is kept for reuse.
                            if (shown) {
                                spare_picture_ = std::move(picture_);
                                picture_ = std::move(image);
                            } else {
                                // The display was off: only the buffer comes back.
                                spare_picture_ = std::move(image);
                            }

                            picture_shown_ = shown;
                        }

                        // Only presents that waited for room in `present` were counted there.
                        if (counted) {
                            {
                                std::lock_guard<std::mutex> lock(mutex_);
                                presents_waiting_--;
                            }
                            done_.notify_all();
                        }
                    }
                );
            });
            break;
    }
}

void Drawing::loop() {
    std::unique_lock<std::mutex> lock(mutex_);

    // Runs until asked to stop with the queue empty, so commands given before the stop are done.
    for (;;) {
        work_.wait(lock, [this] { return quit_ || !queue_.empty(); });

        // Asked to stop, and nothing is left.
        if (queue_.empty()) {
            return;
        }

        Command command = std::move(queue_.front());
        queue_.pop_front();
        busy_ = true;

        // The command runs without the lock, so the EE's thread can queue the next ones.
        lock.unlock();

        run(command);

        lock.lock();
        busy_ = false;

        // Keep the command's buffer for reuse, up to eight of them.
        if (command.data.capacity() && spare_.size() < 8) {
            command.data.clear();
            spare_.push_back(std::move(command.data));
        }
        done_.notify_all();
    }
}

}  // namespace sys
