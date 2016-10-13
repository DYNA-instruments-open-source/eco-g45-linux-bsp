#!/bin/bash

prog_dir=$(dirname $(readlink -f $0))

cd $prog_dir

cat >.dockerignore <<EOign
$(basename $0)
BSP-3.14
cached-distfiles
EOign

tar_args_exclude="--exclude-backups --exclude-vcs --no-xattrs"
tar_args_owner="--numeric-owner --owner=4000 --group=4000"

echo "checking local src distfiles..."
tar_file=cached-distfiles.tar
tar_target=cached-distfiles
test -f $tar_file \
 && md5sum -c ${tar_target}.md5 2>/dev/null 1>&2 </dev/null \
 && { find $tar_target -type f | xargs md5sum; } >${tar_target}.md5 \
 && md5sum -c ${tar_target}.md5.md5 2>/dev/null 1>&2 </dev/null \
 || { 
      echo "recreating local src tar file..."
      { find $tar_target -type f | xargs md5sum; } >${tar_target}.md5
      md5sum ${tar_target}.md5 >${tar_target}.md5.md5
      tar $tar_args_exclude $tar_args_owner -czf $tar_file -C $tar_target .
    } \
 || exit $?

echo "checking BSP files..."
tar_file=DYNA-ECO-G45_BSP_linux-3.14.tar
tar_target=BSP-3.14
test -f $tar_file \
 && md5sum -c ${tar_target}.md5 2>/dev/null 1>&2 </dev/null \
 && { find $tar_target -type f | xargs md5sum; } >${tar_target}.md5 \
 && md5sum -c ${tar_target}.md5.md5 2>/dev/null 1>&2 </dev/null \
 || { 
      echo "recreating BSP tar file..."
      { find $tar_target -type f | xargs md5sum; } >${tar_target}.md5
      md5sum ${tar_target}.md5 >${tar_target}.md5.md5
      tar $tar_args_exclude $tar_args_owner -czf $tar_file $tar_target
    } \
 || exit $?
