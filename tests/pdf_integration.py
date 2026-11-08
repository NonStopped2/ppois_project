"""Real OCR/PDF check. Requires Pillow, rus+eng tessdata, Poppler and a Cyrillic TTF."""

import argparse
import os
from pathlib import Path
import re
import subprocess
import tempfile

from PIL import Image, ImageDraw, ImageFont


def run(*args, success=True):
    env = dict(os.environ, LC_ALL="C")
    result = subprocess.run(args, capture_output=True, text=True, encoding="utf-8", env=env)
    if (result.returncode == 0) != success:
        raise AssertionError(f"Unexpected exit code: {args}\n{result.stdout}\n{result.stderr}")
    return result.stdout


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", required=True)
    parser.add_argument("--font", required=True)
    args = parser.parse_args()
    binary = str(Path(args.binary).resolve())
    font = ImageFont.truetype(args.font, 56)
    with tempfile.TemporaryDirectory(prefix="lecture2pdf-integration-") as folder:
        root = Path(folder)
        inputs = []
        for number, lines in enumerate([
            ["Лекция о программировании", "Инкапсуляция защищает состояние."],
            ["Object oriented programming", "Composition connects objects."],
        ], 1):
            image = Image.new("RGB", (1800, 1000), "white")
            draw = ImageDraw.Draw(image)
            for index, line in enumerate(lines):
                draw.text((100, 200 + 200 * index), line, font=font, fill="black")
            path = root / f"slide{number}.png"
            image.save(path, dpi=(150, 150))
            inputs.append(str(path))
        for filename, paths, fragments in [
            ("normal.pdf", inputs, ["Инкапсуляция", "Composition connects objects"]),
            ("reversed.pdf", inputs[::-1], ["Composition connects objects", "Инкапсуляция"]),
        ]:
            output = str(root / filename)
            run(binary, output, *paths)
            info = run("pdfinfo", output)
            assert re.search(r"^Pages:\s+2\s*$", info, re.MULTILINE), info
            for number, fragment in enumerate(fragments, 1):
                text = run("pdftotext", "-f", str(number), "-l", str(number),
                           "-enc", "UTF-8", output, "-")
                assert " ".join(fragment.casefold().split()) in " ".join(text.casefold().split()), text
        existing = root / "normal.pdf"
        original = existing.read_bytes()
        run(binary, str(existing), *inputs, success=False)
        assert existing.read_bytes() == original
        bad = root / "bad.png"
        bad.write_text("not an image", encoding="utf-8")
        failed = root / "failed.pdf"
        before = set(root.iterdir())
        run(binary, str(failed), inputs[0], str(bad), success=False)
        assert not failed.exists()
        assert set(root.iterdir()) == before, "Staging directory leaked after failure"
        run(binary, str(root / "empty.pdf"), success=False)
    print("PASS: real PDF, rus+eng text, reversed order, no overwrite, failure cleanup")


if __name__ == "__main__":
    main()
