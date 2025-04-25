cmakefile=./CMakeLists.txt
windir=/mnt/d/ESP/flash_download_tool/temp
builddir=./build

#! 清空windows端目标文件夹
rm -f ${windir}/**.bin
echo 目标文件夹: ${windir} 已清空

#! 提取项目名称
target=$(grep -E "^project\(" "$cmakefile" | sed -E 's/project\(([^)]+)\)/\1/')
target=$(echo $target | tr -d '\r')
echo 项目名称: $target

#! 从第二行开始, 读取第二列参数
flash_args=$(awk 'NR>1 {print $2}' $builddir/flash_args)
for filepath in $flash_args
do
  filename=$(basename "$filepath")
  if [ "$filename" = "$target.bin" ];then
    wfilename="main.bin"
  else
    wfilename=$filename
  fi
  filepath=${builddir}/${filepath}
  wfilepath=${windir}/${wfilename}
  if [ -f $filepath ]; then
    cp $filepath ${wfilepath}
    echo 已复制 $filename '>>' $wfilename
  fi
done