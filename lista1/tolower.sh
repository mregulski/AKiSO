OLD_IFS=$IFS
export IFS=$(printf "\n\t");
for f in `find -type f`; do
  name2=`echo $f | tr [:lower:] [:upper:]`;
  mv "$f" "$name2";
done
export IFS=$OLD_IFS;
