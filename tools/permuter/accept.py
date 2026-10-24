# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2026 the OpenRAC contributors
"""The acceptance rule for permuter results (WORKFLOW.md: accept only plausible C), as far as a
program can apply it: the artefacts a candidate has and its base does not.

  duplicated call   the same call statement in both branches of an if
  dead store        a local assigned and then assigned again before anything reads it, or
                    assigned and never read at all
  uninitialised     a local without an initialiser whose first use in the function reads it

Each is found on decomp-permuter's own parse of the candidate (perm_pycparser, from the checkout
upstream.py loads), so a candidate it cannot parse reports nothing. What is left to the reader is
in the README: changed arguments, narrowed types, dropped stores, behaviour in general.
"""
from __future__ import annotations


def _modules():
    from perm_pycparser import c_ast, c_generator
    from perm_pycparser.c_parser import CParser
    return c_ast, c_generator.CGenerator(), CParser()


def _function(ast, c_ast, func):
    for node in ast.ext:
        if isinstance(node, c_ast.FuncDef) and node.decl.name == func:
            return node
    return None


def _walk(node):
    yield node
    for _, child in node.children():
        yield from _walk(child)


def _ids(node, c_ast) -> set[str]:
    return {n.name for n in _walk(node) if isinstance(n, c_ast.ID)} if node is not None else set()


def _store(stmt, c_ast) -> str | None:
    """The local a statement `x = e;` stores to, when it is that."""
    if isinstance(stmt, c_ast.Assignment) and stmt.op == "=" and isinstance(stmt.lvalue, c_ast.ID):
        return stmt.lvalue.name
    return None


def find(source: str, func: str) -> set[str]:
    """The artefacts in FUNC as SOURCE defines it, each a line of text; empty when it does not parse."""
    try:
        c_ast, gen, parser = _modules()
        fn = _function(parser.parse(source), c_ast, func)
    except Exception:
        return set()
    if fn is None:
        return set()
    found = set()
    locals_ = {d.name: d for d in _walk(fn.body) if isinstance(d, c_ast.Decl) and d.name
               and not isinstance(d.type, c_ast.FuncDecl)}
    # duplicated calls in both branches
    for node in _walk(fn.body):
        if isinstance(node, c_ast.If) and node.iftrue is not None and node.iffalse is not None:
            calls = [{gen.visit(n) for n in _walk(side) if isinstance(n, c_ast.FuncCall)}
                     for side in (node.iftrue, node.iffalse)]
            for call in sorted(calls[0] & calls[1]):
                found.add(f"duplicated call: {call} in both branches of `if ({gen.visit(node.cond)})`")
    # dead stores: overwritten before a read in the same block, or never read
    uses, stored = {}, {}
    for node in _walk(fn.body):
        if isinstance(node, c_ast.ID):
            uses[node.name] = uses.get(node.name, 0) + 1
        name = _store(node, c_ast)
        if name:
            stored[name] = stored.get(name, 0) + 1
    for name in locals_:
        if stored.get(name) and uses.get(name, 0) == stored[name]:
            found.add(f"dead store: {name} is assigned and never read")
    for node in _walk(fn.body):
        if not isinstance(node, c_ast.Compound) or not node.block_items:
            continue
        items = node.block_items
        if any(isinstance(n, (c_ast.Label, c_ast.Goto)) for item in items for n in _walk(item)):
            continue
        for i, stmt in enumerate(items):
            name = _store(stmt, c_ast)
            if name not in locals_:
                continue
            for later in items[i + 1:]:
                if _store(later, c_ast) == name and name not in _ids(later.rvalue, c_ast):
                    found.add(f"dead store: `{gen.visit(stmt)}` is overwritten before it is read")
                    break
                if name in _ids(later, c_ast):
                    break
    # uninitialised reads: the first use of a local declared without a value is a read
    for name, decl in locals_.items():
        if decl.init is not None or isinstance(decl.type, c_ast.ArrayDecl) or "static" in (decl.storage or []):
            continue
        for node in _walk(fn.body):
            if isinstance(node, c_ast.Assignment) and isinstance(node.lvalue, c_ast.ID) \
                    and node.lvalue.name == name and node.op == "=":
                if name in _ids(node.rvalue, c_ast):
                    found.add(f"uninitialised read: {name} is read before it is set")
                break
            if isinstance(node, c_ast.UnaryOp) and node.op == "&" and name in _ids(node.expr, c_ast):
                break
            if isinstance(node, c_ast.ID) and node.name == name:
                found.add(f"uninitialised read: {name} is read before it is set")
                break
    return found


def artefacts(base: str, candidate: str, func: str) -> list[str]:
    """The artefacts CANDIDATE has that BASE (the run's starting source) does not, sorted."""
    return sorted(find(candidate, func) - find(base, func))
