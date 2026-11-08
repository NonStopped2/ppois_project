# Сборка и проверка модулей Арсения

## Core: Linux, Windows, macOS

Нужны Git, CMake >= 3.20, компилятор с C++17. GoogleTest v1.15.2
подключается из установленного CMake-пакета либо скачивается на фиксированном
коммите при первой конфигурации (нужен интернет). Tesseract для core не нужен.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug --parallel 2
ctest --test-dir build -C Debug --output-on-failure
```

На Windows команды выполнять из Developer PowerShell с установленным Visual
Studio Build Tools и компонентом Desktop development with C++. В Linux обычно
достаточно пакетов build-essential, cmake, git. На macOS — Command Line Tools и CMake.

Для сборки только библиотеки без загрузки GoogleTest: `-DBUILD_TESTING=OFF`.
Для гарантированного отключения ранее включённого адаптера добавьте
`-DLECTURE2PDF_WITH_TESSERACT=OFF` или используйте новый каталог сборки.

## Необязательный PDF-адаптер Tesseract

macOS:

```sh
brew install cmake pkg-config tesseract tesseract-lang poppler
```

Ubuntu / Debian:

```sh
sudo apt install build-essential cmake git pkg-config libtesseract-dev libleptonica-dev tesseract-ocr-eng tesseract-ocr-rus poppler-utils
```

```sh
tesseract --list-langs
cmake -S . -B build-pdf -DLECTURE2PDF_WITH_TESSERACT=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build-pdf --parallel 2
ctest --test-dir build-pdf --output-on-failure
./build-pdf/lecture2pdf_pdf_demo lecture.pdf slide1.jpg slide2.jpg
```

Нужны языки rus и eng; tessdata содержит также pdf.ttf. При необходимости
задайте TESSDATA_PREFIX равным каталогу tessdata вашей установки.
CLI — диагностический пример, не готовый модуль OCR Егора.

Нативная Windows-сборка PDF-адаптера пока не настроена: он использует pkg-config
и узкосимвольный файловый API Tesseract/Leptonica. Для эксперимента используйте WSL.
Core не зависит от этого ограничения.

## Автоматическая интеграционная проверка

Дополнительно нужны Python 3, Pillow, Poppler и TTF-шрифт с кириллицей.
Pillow устанавливайте в своём Python-окружении; в Ubuntu доступен пакет python3-pil.

macOS (при наличии Pillow):

```sh
python3 tests/pdf_integration.py --binary build-pdf/lecture2pdf_pdf_demo --font '/System/Library/Fonts/Supplemental/Arial.ttf'
```

Linux:

```sh
/usr/bin/python3 tests/pdf_integration.py --binary build-pdf/lecture2pdf_pdf_demo --font /usr/share/fonts/truetype/dejavu/DejaVuSans.ttf
```

Скрипт создаёт собственные синтетические изображения во временном каталоге,
проверяет русский и английский текст на нужных страницах, обратный порядок,
запрет перезаписи результата и очистку при сбое декодирования. Чужие фотографии
и персональные данные не нужны. Это отдельная проверка, не часть unit-тестов CTest.

Для визуальной проверки откройте PDF, найдите слово, выделите и скопируйте текст.
Автоматическая проверка не гарантирует идеального OCR или выравнивания выделения.

## CI и фактическая проверка

GitHub Actions настроен на core для Linux, Windows и macOS, а также реальный
PDF/OCR-тест на Linux. Наличие workflow не означает, что он уже прошёл в GitHub.
Локально проверена macOS; результаты CI нужно смотреть после отправки ветки.

Каталог build и пользовательские изображения не коммитим. Примеры PDF создавайте
в build, а не в корне репозитория. Существующий результат не перезаписывается.
