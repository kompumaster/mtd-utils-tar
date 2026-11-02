# Сборка под Windows

Форк mtd-utils 2.3.1: `mkfs.ubifs_tar.exe` собирается MinGW-w64, без
Cygwin и без WSL. В `mkfs.ubifs` добавлен режим `--tar`, поэтому
дерево берётся из архива, а не из файловой системы.

Состояние: Linux- и Windows-сборки работают, сжатие zlib/lzo/zstd
включено, `--tar` и `--tar-skip` проверены. Подробности по
Windows-слою — `win32/README.md`.

## Что где

- `ubifs-utils/mkfs.ubifs/mkfs.ubifs.c` — режим `--tar` и
  `--tar-skip`;
- `ubifs-utils/mkfs.ubifs/tar_index.{c,h}` — разбор tar-архива;
- `win32/compat/` — POSIX-заглушки для MinGW, подключаются через
  `-include`;
- `win32/build-*.sh` — сборка;
- `win32/third_party/` — закоммиченные архивы zlib, lzo и zstd.

`ubinize`, `fsck.ubifs` и остальной mtd-utils не менялись.

## Сборка

Сначала статические библиотеки:

```sh
./win32/build-zlib.sh
./win32/build-lzo.sh
./win32/build-zstd.sh
```

Затем сам бинарник:

```sh
./win32/build-mingw.sh     # Windows: win32/mkfs.ubifs_tar.exe
./win32/build-linux.sh     # Linux:   win32/mkfs.ubifs_tar
```

Архивы из `win32/third_party/` уже лежат в репозитории; скрипты
распаковывают их и не обращаются к сети, если архив на месте.

Windows-бинарник линкуется статически с libgcc и winpthread, поэтому
ему нужен только UCRT — он есть в Windows 10 и новее.

## Проверка

- `mkfs.ubifs_tar --version` под wine печатает
  `mkfs.ubifs_tar (mtd-utils) 2.3.1`;
- `--tar` с `--tar-skip` и без него собирает образ;
- обычный режим `-r DIR` продолжает работать;
- симлинки, hardlinks, каталоги, файлы, mode/uid/gid сохраняются;
- на реальном `system_b` круг «раздел → tar → раздел» дал 0 отличий.

## Ограничения

- `--tar` поддерживает ustar, GNU longname и основные поля pax;
  контрольная сумма архива не проверяется — архив считается
  доверенным;
- без `--tar` на Windows mode/uid/gid неверные: NTFS их не хранит;
- zstd-образы читает только ядро 5.9 и новее, по умолчанию — zlib;
- OpenSSL и криптография в Windows-сборку не входят
  (`WITH_CRYPTO` не включается).

## Автоматическая сборка

Штатная autotools-сборка (Linux) описана в `README.txt`:
`./autogen.sh && ./configure && make mkfs.ubifs`. Скрипты
`win32/build-*.sh` — отдельный ручной путь для Linux и Windows: они
собирают только `mkfs.ubifs` и не требуют `configure`.
