
 mtd-utils
 =========

 Форк: mkfs.ubifs с режимом --tar
 =================================

 Это форк mtd-utils 2.3.1. В утилитах добавлен один режим:
 `mkfs.ubifs` умеет собирать образ UBIFS из tar-архива, не
 распаковывая дерево на диск.

 Зачем:

 - дерево файловой системы нельзя разложить на NTFS: имена могут
   различаться только регистром, есть симлинки, setuid и разные
   uid/gid;
 - tar хранит mode, uid, gid, mtime, симлинки и hardlinks;
 - для Windows-сборки не нужны Cygwin и WSL;
 - отдельный проект UbiTar (https://github.com/kompumaster/UbiTar)
   разбирает UBI в tar и собирает обратно под Windows и Linux.

 Что добавлено:

 - `--tar=FILE` — читать дерево из архива;
 - `--tar-skip=PATH` — не включать путь и его потомков в образ;
   флаг можно повторять, по умолчанию не пропускается ничего;
 - `ubifs-utils/mkfs.ubifs/tar_index.{c,h}` — разбор ustar, GNU
   longname и основных полей pax;
 - `win32/compat/` — POSIX-заглушки для MinGW-w64;
 - `win32/build-*.sh` — сборка Linux и Windows;
 - `win32/third_party/` — архивы zlib, lzo и zstd.

 Что не менялось: остальной mtd-utils, включая `ubinize`,
 `fsck.ubifs` и библиотеки. Программа называется `mkfs.ubifs_tar`,
 чтобы не путаться со штатной `mkfs.ubifs`.

 Сборка через autotools:

     ./autogen.sh
     ./configure
     make mkfs.ubifs

 Сборка для Windows:

     ./win32/build-zlib.sh
     ./win32/build-lzo.sh
     ./win32/build-zstd.sh
     ./win32/build-mingw.sh

 Сборка для Linux без autotools:

     ./win32/build-linux.sh

 Подробности — `WINDOWS.md` и `win32/README.md`.

 Проверено: `--tar` с `--tar-skip` и без, обычный режим `-r`,
 симлинки, hardlinks, каталоги и файлы; Windows-сборка запускается
 под wine и собирает образ; на реальном `system_b` круг
 «раздел → tar → раздел» дал 0 отличий.

 Ограничения:

 - `--tar` рассчитан на ustar, GNU longname и основные поля pax;
   контрольная сумма архива не проверяется — архив считается
   доверенным;
 - без `--tar` на Windows mode/uid/gid неверные: NTFS их не хранит;
 - zstd-образы читает только ядро 5.9 и новее, по умолчанию — zlib;
 - Windows-бинарник требует UCRT (Windows 10 и новее).

 Лицензия: GPL-2.0, см. `COPYING`.

 This package provides userspace utilities for working with Linux MTD and UBI
 infrastructure, as well as related filesystems (JFFS2, UBIFS).

 MTD stands for "Memory Technology Devices" and means flash, RAM and similar
 chips, used for solid state storage on embedded devices. Think of e.g. a raw
 nand-flash chip attached directly to an SoC via parallel or SPI bus, but
 without _any_ special management like an eMMC has. The MTD subsystem exposes
 those devices to userspace as character devices. It provides a unified
 interface, but does not attempt to hide the page vs erase-block device
 geometry or issues involved in dealing with the underlying storage, like wear
 or bad blocks.

 For this purpose, UBI provides a layer on top of MTD that hides away some
 of those complexities. UBI also implements LVM-like logical volume partitioning
 and some other useful capabilities.

 UBI does not emulate block device like storage. UBI volumes are exposed as
 character devices with page vs erase-block access geometry, much like MTD
 partitions. But they behave more like a flash chip with "idealized" properties
 and transparent wear-leveling.

 JFFS2 is a flash friendly filesystem. It predates UBI and is designed to run
 on top of raw MTD, implementing its own wear leveling.

 The more recent UBIFS (formerly JFFS3) can be used on top of UBI volumes and
 offers a number of improvements over JFFS2, offers newer compression schemes
 and advanced features like file-level encryption and authentication.


 Installing
 ----------

 mtd-utils are packaged by a number of Linux distributions[0] and can be
 installed via the package management system. A Yocto recipe is available
 as well[1] as part of the openembedded-core layer.

 Should the tools be too big for on-device use, the BusyBox project provides
 compatible clone versions.

 Before making this choice however, please note that some "heavy weight" tools
 like mkfs.ubifs are typically not needed on the device itself. The kernel
 module can format a volume on first mount. Other, heavy weight tools like lsmtd
 can be disabled via a configure flag.

 [0] https://repology.org/project/mtd-utils/versions
 [1] https://layers.openembedded.org/layerindex/recipe/262/


 How to get the source
 ---------------------

 The official git repository and release tarballs are hosted on infradead.org:

   Relase tarballs:  ftp://ftp.infradead.org/pub/mtd-utils
   git repository:   git://git.infradead.org/mtd-utils.git

 This is also where the website and documentation are hosted:

   http://www.linux-mtd.infradead.org/

 A http mirror is also provided on infraroot.at:

   Relase tarballs:  https://infraroot.at/pub/mtd
   git repository:   https://git.infraroot.at/mtd-utils.git


 There are a number of mirrored repositories on GitHub, but this is not where
 official development takes place. If you want to contribute upstream, please
 do not open any issues or pull requests there. Upstream development is done
 via mailing list (see below).


 How to build
 ------------

 mtd-utils uses an autotools based build system. If you are building the git
 tree, you first need to run `./autogen.sh`. This sets up the build system
 and requires autoconf, automake and possibly other autotools to be installed.

 After unpacking a tarball (or running autogen.sh) simply run:

 $ ./configure
 $ make

 to build the package. This does not require autotools to be installed, but
 does need pkg-config, make, etc...

 There are a number of configure switches to tune the build.
 Run `./configure --help` to get an overview.


 For compiling mtd-utils, you need development packages for the following
 dependency libraries:

   zlib          (optional dependency for mkfs.ubifs, mkfs.jffs2)
   lzo2          (optional dependency for mkfs.ubifs, mkfs.jffs2)
   zstd          (optional dependency for mkfs.ubifs)
   libuuid*      (required by mkfs.ubifs)
   libselinux    (optional dependency for mkfs.ubifs)
   openssl       (optional dependency for mkfs.ubifs)

 * this library is part of util-linux, aka util-linux-ng and some distributions
   package it under that name.

 For xattr & acl support in mkfs.ubifs and mkfs.jffs2, the build system looks
 for the "sys/xattr.h" and "sys/acl.h" header files. Depending on your
 distributions, those may be packaged as part of libattr and libacl
 respectively.


 Please note that the mkfs tools are optional and can be disabled via a
 configure flag. This should also remove any library dependencies.


 How to contribute
 -----------------

 Development of mtd-utils takes place on the linux-mtd mailing list:

  mailto:   linux-mtd@lists.infradead.org
  archive:  https://lists.infradead.org/pipermail/linux-mtd/

 You can subscribe here: http://lists.infradead.org/mailman/listinfo/linux-mtd/
 or simply send "subscribe" to linux-mtd-request@lists.infradead.org


 Contributions are submitted in the form of plain text patches,
 using `git send-email` or similar.

 Please prefix your subject with "mtd-utils: " to make them easier to spot.

 It may take a few days for a patch to be picked up. This is in part done
 intentionally to allow other people to comment on it. If it's been more
 than a week and you feel your patch might have been overlooked, please send
 a friendly ping, or re-submit the patch series with "[RESEND]" in the subject.


 There is also a #mtd IRC channel on irc.oftc.net
