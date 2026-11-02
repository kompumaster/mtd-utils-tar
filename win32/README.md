# win32: сборка mkfs.ubifs под Windows

Сборка MinGW-w64, без Cygwin и без WSL. Режим `--tar` живёт в
`ubifs-utils/mkfs.ubifs/`, а здесь — только Windows-обвязка:
POSIX-заглушки, статические библиотеки и скрипты сборки.

## Сборка

```sh
./win32/build-zlib.sh      # статическая zlib
./win32/build-lzo.sh       # статическая lzo2
./win32/build-zstd.sh      # статическая zstd
./win32/build-mingw.sh     # Windows: win32/mkfs.ubifs_tar.exe
./win32/build-linux.sh     # Linux:   win32/mkfs.ubifs_tar
```

Результат: `win32/mkfs.ubifs_tar.exe` (Windows) или
`win32/mkfs.ubifs_tar` (Linux). Оба имени отличаются от штатной
`mkfs.ubifs`, чтобы не путаться с системной утилитой.

Windows-бинарник собран с `-static`: libgcc и winpthread вкомпилированы,
поэтому нужен только UCRT (Windows 10 и новее).

## Что добавлено

| Файл | Зачем |
|---|---|
| `compat/compat.h` | объявления и недостающие константы (`S_IFLNK`, `S_ISGID`, `O_LARGEFILE`, `EUCLEAN`, `VERSION`) |
| `compat/compat.c` | `pread`/`pwrite`, `getline`, `readlink`, `lstat`, `dirfd`, `ioctl`, `xasprintf`, `realpath`, `getpagesize`, `uuid`; перевод файлов в двоичный режим |
| `compat/include/` | заголовки-заглушки: `linux/*`, `asm/byteorder.h`, `byteswap.h`, `endian.h`, `features.h`, `sys/ioctl.h`, `sys/sysmacros.h`, `uuid.h` |
| `compat/linux_build.h` | то, что autotools обычно передаёт через `config.h`, для ручной сборки |
| `compat/ubi_stub.c` | заглушки UBI/MTD: запись в `/dev/ubi*` не поддерживается, пишем в файл |
| `third_party/` | архивы zlib, lzo и zstd с версиями и SHA-256 |
| `build-zlib.sh`, `build-lzo.sh`, `build-zstd.sh` | статические библиотеки для Windows |
| `build-mingw.sh`, `build-linux.sh` | сборка `mkfs.ubifs_tar` |

Компилируются только нужные части mtd-utils: `ubifs-utils/common`,
`ubifs-utils/libubifs` (без `auth.c`), `mkfs.ubifs.c`,
`tar_index.c`, `lib/common.c`, `libcrc32.c`, `rbtree.c`,
`list_sort.c`, `execinfo.c`. OpenSSL и криптография не нужны
(`WITH_CRYPTO` не включается).

## Состояние

- собирается и запускается под wine: `mkfs.ubifs_tar (mtd-utils) 2.3.1`;
- образ из тестового дерева совпадает с Linux-сборкой по содержимому,
  размеру и структуре; различаются только UUID и время;
- сжатие zlib, lzo и zstd включено (`build-zlib.sh`, `build-lzo.sh`,
  `build-zstd.sh`, статические `libz.a`, `liblzo2.a`, `libzstd.a`);
  zstd-образы монтирует только ядро 5.9+, поэтому по умолчанию не он;
- режим `--tar FILE`: дерево берётся из архива, `mode`/`uid`/`gid`,
  симлинки и hardlinks сохраняются точно;
- `--tar-skip=PATH` (можно повторять) исключает путь и его потомков;
  без флага не пропускается ничего;
- проверено на реальном `system_b`: 2693 записи, 0 отличий от tar
  (содержимое, права, владельцы, симлинки), 2,4 с под wine;
- с файловой системой Windows (без `--tar`) `mode`/`uid`/`gid`
  неверные — это ожидаемо, для работы нужен режим `--tar`.

## Ограничения

- `--tar` поддерживает ustar, GNU longname и основные поля pax;
  контрольная сумма архива не проверяется — архив считается
  доверенным;
- Windows-бинарник требует UCRT;
- OpenSSL и криптография не входят;
- `--tar-skip` не имеет значений по умолчанию: служебные каталоги
  нужно указывать явно.

## Дальше

Со стороны Windows-слоя работа закончена. Сборка раздела из tar и
обратно — задача проекта UbiTar
(https://github.com/kompumaster/UbiTar). После перехода на этот форк
UbiTar должен передавать `--tar-skip` для своих служебных каталогов:
жёсткого пропуска больше нет.
