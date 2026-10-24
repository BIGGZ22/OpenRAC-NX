# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""Verify lossless shader embedding, bounded literals and repeatable SDK preparation."""

import re
import tempfile
import unittest
from pathlib import Path

import prepare_sdk


# The scratch layout of the backend: six arrays sized twice, then mapped from the context's end.
SCRATCH = "".join(
    f"uint32_t {name}ArraySize = FFX_ALIGN_UP(count * sizeof(T), sizeof(uint32_t));\n"
    for name in ("gpuJobDesc", "resourceView", "stagingRingBuffer", "pipeline", "resource",
                 "context") * 2
) + (
    "return FFX_ALIGN_UP(sizeof(BackendContext_VK) + extensionPropArraySize, 8);\n"
    "uint8_t* pMem = (uint8_t*)((BackendContext_VK*)(backendContext + 1));\n"
)


class PrepareSdkTest(unittest.TestCase):
    """Synthetic source files exercise preparation without requiring the downloaded SDK."""

    def setUp(self):
        self.folder = tempfile.TemporaryDirectory()
        self.addCleanup(self.folder.cleanup)
        self.root = Path(self.folder.name)
        self.source = self.root / "source"
        self.output = self.root / "output"
        self.gpu = self.source / "sdk/include/FidelityFX/gpu"
        self.gpu.mkdir(parents=True)
        context = self.source / 'sdk/include/FidelityFX/host/ffx_fsr3upscaler.h'
        context.parent.mkdir(parents=True)
        context.write_text(
            '// Synthetic context fixture.\n'
            '#define FFX_FSR3UPSCALER_CONTEXT_SIZE (FFX_SDK_DEFAULT_CONTEXT_SIZE)\n',
            encoding='utf-8',
        )
        backend = self.source / "sdk/src/backends/vk/ffx_vk.cpp"
        backend.parent.mkdir(parents=True)
        backend.write_text(
            "// Synthetic backend fixture.\nuint32_t findMemoryTypeIndex() {}\n"
            "VkBufferUsageFlags fixture;\n"
            'const char* symbol = "vkGetBufferMemoryRequirements2KHR";\n'
            "backendInterface->fpSwapChainConfigureFrameGeneration = "
            "ffxSetFrameGenerationConfigToSwapchainVK;\n" + SCRATCH,
            encoding="utf-8",
        )

    def test_long_source_is_reconstructed_exactly_and_literals_are_bounded(self):
        text = '// Unicode and quotes survive: "é"\n' + "a" * 40000
        (self.gpu / "fixture.h").write_text(text, encoding="utf-8")
        prepare_sdk.prepare(self.source, self.output)
        header = (self.output / "fsr3_sources.h").read_text(encoding="utf-8")
        chunks = re.findall(r'R"fsrsdk\((.*?)\)fsrsdk"', header, re.S)
        self.assertEqual("".join(chunks), text)
        self.assertTrue(all(len(chunk) <= 16384 for chunk in chunks))

    def test_unchanged_preparation_does_not_rewrite_generated_files(self):
        (self.gpu / "fixture.h").write_text("// fixture\n", encoding="utf-8")
        prepare_sdk.prepare(self.source, self.output)
        before = {path.relative_to(self.output): (path.read_bytes(), path.stat().st_mtime_ns)
                  for path in self.output.rglob('*') if path.is_file()}
        prepare_sdk.prepare(self.source, self.output)
        after = {path.relative_to(self.output): (path.read_bytes(), path.stat().st_mtime_ns)
                 for path in self.output.rglob('*') if path.is_file()}
        self.assertEqual(before, after)

    def test_context_storage_accounts_for_native_wide_character_size(self):
        prepare_sdk.prepare(self.source, self.output)
        context = (self.output / 'FidelityFX/host/ffx_fsr3upscaler.h').read_text(encoding='utf-8')
        self.assertIn('sizeof(wchar_t) / sizeof(uint16_t)', context)
        self.assertTrue(context.startswith('// Synthetic context fixture.'))

    def test_scratch_arrays_are_aligned_for_their_types(self):
        prepare_sdk.prepare(self.source, self.output)
        backend = (self.output / "ffx_vk.cpp").read_text(encoding="utf-8")
        self.assertNotIn("sizeof(uint32_t))", backend)
        self.assertEqual(backend.count(", 64);"), 12)
        self.assertIn("FFX_ALIGN_UP((uintptr_t)(backendContext + 1), (uintptr_t)64)", backend)
        self.assertIn("sizeof(BackendContext_VK) + 64 + extensionPropArraySize", backend)

    def test_an_unexpected_scratch_layout_is_rejected(self):
        backend = self.source / "sdk/src/backends/vk/ffx_vk.cpp"
        backend.write_text(backend.read_text(encoding="utf-8").replace("pMem", "memory"),
                           encoding="utf-8")
        with self.assertRaisesRegex(ValueError, "scratch layout"):
            prepare_sdk.prepare(self.source, self.output)

    def test_raw_string_collision_is_rejected(self):
        (self.gpu / "fixture.h").write_text(')fsrsdk"', encoding="utf-8")
        with self.assertRaisesRegex(ValueError, "delimiter"):
            prepare_sdk.prepare(self.source, self.output)


if __name__ == "__main__":
    unittest.main()
