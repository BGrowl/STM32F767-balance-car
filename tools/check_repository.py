"""Check source-package integrity using only the Python standard library."""

import ast
from pathlib import Path
import re
import sys
from urllib.parse import unquote, urlsplit
import xml.etree.ElementTree as ET


ROOT = Path(__file__).resolve().parents[1]


def main():
    errors = []
    required = [
        "README.md", "LICENSE", "THIRD_PARTY_NOTICES.md", "CONTRIBUTING.md",
        "docs/hardware.md", "docs/getting-started.md", "docs/control.md",
        "docs/protocol.md", "docs/k230.md", "docs/validation.md",
        "STM32F767-balance-car.ioc", "K230_Part.py",
        "Core/Src/main.c", "Drivers/BSP/PID/pid.c",
        "Drivers/BSP/MPU6050/eMPL/inv_mpu.c",
        "Drivers/CMSIS/LICENSE.txt",
        "Drivers/CMSIS/Device/ST/STM32F7xx/LICENSE.txt",
        "Drivers/STM32F7xx_HAL_Driver/LICENSE.txt",
        "Middlewares/ST/STM32_USB_Device_Library/LICENSE.txt",
    ]
    for name in required:
        if not (ROOT / name).is_file():
            errors.append(f"Missing required file: {name}")

    project = ROOT / "MDK-ARM/STM32F767-balance-car.uvprojx"
    tree = ET.parse(project)
    sources = tree.findall(".//FilePath")
    for element in sources:
        name = element.text.replace("\\", "/")
        path = (project.parent / name).resolve()
        if not path.is_relative_to(ROOT) or not path.is_file():
            errors.append(f"Missing or external Keil source: {name}")

    documents = list(ROOT.glob("*.md")) + list((ROOT / "docs").glob("*.md"))
    links = 0
    for document in documents:
        content = document.read_text(encoding="utf-8")
        for target in re.findall(r"!?\[[^\]]*\]\(([^)]+)\)", content):
            target = target.strip("<>")
            parsed = urlsplit(target)
            if parsed.scheme or not parsed.path:
                continue
            links += 1
            resolved = (document.parent / unquote(parsed.path)).resolve()
            if not resolved.is_relative_to(ROOT) or not resolved.exists():
                errors.append(f"Broken local link in {document.name}: {target}")

    ast.parse((ROOT / "K230_Part.py").read_text(encoding="utf-8"))
    ast.parse((ROOT / "tools/render_balance_car_schematic.py").read_text(encoding="utf-8"))
    if errors:
        print("\n".join(errors))
        return 1
    print(f"PASS: {len(required)} required files, {len(sources)} Keil source paths, "
          f"{len(documents)} Markdown documents, {links} local links, 2 Python syntax checks")
    print("Scope: package integrity only; this does not compile or test hardware.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
