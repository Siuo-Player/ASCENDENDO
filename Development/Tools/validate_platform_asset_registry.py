#!/usr/bin/env python3
"""Validate exact platform-asset identity declared by the canonical registry.

The registry intentionally permits an explicit UNPOPULATED state while an
external binary has not yet been staged. Once identity is populated, this
validator requires an asset ID, a repository-relative path, a 64-hex SHA-256,
an existing file, a matching content digest, unique exact identity across
entries, and at most one declaration of each identity field per entry.
"""

from __future__ import annotations

import hashlib
import re
import sys
from pathlib import Path

REGISTRY = Path("Game/Assets/Sprites/PLATFORM_ASSET_REGISTRY.md")
SHA256_RE = re.compile(r"^[0-9a-fA-F]{64}$")
FIELD_RE = re.compile(r"^[-*]\s+`(asset_id|runtime_path|content_sha256)`:\s+`([^`]*)`\s*$")
IDENTITY_LINE_RE = re.compile(r"^[-*]\s+.*\b(asset_id|runtime_path|content_sha256)\b.*:")
SECTION_RE = re.compile(r"^##\s+(.+?)\s*$")
WINDOWS_DRIVE_RE = re.compile(r"^[A-Za-z]:")
UNPOPULATED = "UNPOPULATED"
UNPOPULATED_PREFIX = "UNPOPULATED — "
IDENTITY_FIELDS = ("asset_id", "runtime_path", "content_sha256")


def is_unpopulated(value: str | None) -> bool:
    return value is None or value == UNPOPULATED or value.startswith(UNPOPULATED_PREFIX)


def path_is_safe(value: str) -> bool:
    path = Path(value)
    if (
        not value
        or path.is_absolute()
        or "\\" in value
        or WINDOWS_DRIVE_RE.match(value) is not None
    ):
        return False
    parts = value.split("/")
    return all(part not in ("", ".", "..") for part in parts)


def validate_registry(text: str, repo_root: Path) -> list[str]:
    errors: list[str] = []
    seen_asset_ids: dict[str, str] = {}
    seen_runtime_paths: dict[str, str] = {}
    seen_hashes: dict[str, str] = {}
    current_section = "<root>"
    asset_id: str | None = None
    runtime_path: str | None = None
    content_sha256: str | None = None
    field_counts = {field: 0 for field in IDENTITY_FIELDS}
    malformed_identity_lines: set[int] = set()

    def reset_entry() -> None:
        nonlocal asset_id, runtime_path, content_sha256
        asset_id = None
        runtime_path = None
        content_sha256 = None
        for field in IDENTITY_FIELDS:
            field_counts[field] = 0

    def flush() -> None:
        nonlocal asset_id, runtime_path, content_sha256
        if asset_id is None and runtime_path is None and content_sha256 is None:
            reset_entry()
            return

        label = current_section
        for field in IDENTITY_FIELDS:
            if field_counts[field] > 1:
                errors.append(f"{label}: duplicate {field} declaration")

        if not asset_id:
            errors.append(f"{label}: asset_id is required")
        elif asset_id in seen_asset_ids:
            errors.append(
                f"{label}: duplicate asset_id {asset_id}; "
                f"already declared by {seen_asset_ids[asset_id]}"
            )
        else:
            seen_asset_ids[asset_id] = label

        path_unpopulated = is_unpopulated(runtime_path)
        hash_unpopulated = is_unpopulated(content_sha256)
        if path_unpopulated and hash_unpopulated:
            reset_entry()
            return

        if path_unpopulated != hash_unpopulated:
            errors.append(f"{label}: runtime_path and content_sha256 must be populated together")
            reset_entry()
            return

        assert runtime_path is not None and content_sha256 is not None
        if not path_is_safe(runtime_path):
            errors.append(f"{label}: invalid repository-relative runtime_path: {runtime_path}")
            reset_entry()
            return
        if SHA256_RE.fullmatch(content_sha256) is None:
            errors.append(f"{label}: content_sha256 must be exactly 64 hexadecimal characters")
            reset_entry()
            return

        normalized_hash = content_sha256.lower()
        previous_path_owner = seen_runtime_paths.get(runtime_path)
        if previous_path_owner is not None:
            errors.append(
                f"{label}: duplicate runtime_path {runtime_path}; "
                f"already declared by {previous_path_owner}"
            )
        else:
            seen_runtime_paths[runtime_path] = label

        previous_hash_owner = seen_hashes.get(normalized_hash)
        if previous_hash_owner is not None:
            errors.append(
                f"{label}: duplicate content_sha256 {content_sha256}; "
                f"already declared by {previous_hash_owner}"
            )
        else:
            seen_hashes[normalized_hash] = label

        target = repo_root / Path(runtime_path)
        if not target.is_file():
            errors.append(f"{label}: runtime_path does not exist: {runtime_path}")
        else:
            digest = hashlib.sha256(target.read_bytes()).hexdigest()
            if digest.lower() != normalized_hash:
                errors.append(
                    f"{label}: SHA-256 mismatch for {runtime_path}: "
                    f"declared {content_sha256}, actual {digest}"
                )

        reset_entry()

    for line_number, line in enumerate(text.splitlines(), start=1):
        match = SECTION_RE.match(line)
        if match:
            flush()
            current_section = match.group(1)
            continue

        match = FIELD_RE.match(line)
        if match:
            field = match.group(1)
            field_counts[field] += 1
            if field == "asset_id":
                asset_id = match.group(2)
            elif field == "runtime_path":
                runtime_path = match.group(2)
            else:
                content_sha256 = match.group(2)
            continue

        if IDENTITY_LINE_RE.match(line) and line_number not in malformed_identity_lines:
            errors.append(f"{current_section}: malformed identity field declaration on line {line_number}")
            malformed_identity_lines.add(line_number)

    flush()
    return errors


def main() -> int:
    if not REGISTRY.is_file():
        print(f"ERROR: canonical registry not found: {REGISTRY}")
        return 1

    errors = validate_registry(REGISTRY.read_text(encoding="utf-8"), Path("."))
    if errors:
        for error in errors:
            print(f"ERROR: {error}")
        return 1

    print("OK: canonical platform asset registry identity is internally consistent")
    return 0


if __name__ == "__main__":
    sys.exit(main())
