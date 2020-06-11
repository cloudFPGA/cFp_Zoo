#/bin/sh

template_kernel="harris"
new_kernel="safe"

#echo "Copying folder " ${template_kernel} "to" ${new_kernel}
#cp -r ./hls/${template_kernel} "to" ./hls/${new_kernel}


find ./ -type d -name ${template_kernel} -print 2>/dev/null | while read dir_src_kernel; do
    parentdir="$(dirname "$dir_src_kernel")"
    newdir=${parentdir}"/"${new_kernel} 
    echo  "Copying dir: " ${dir_src_kernel} " to " ${newdir}
    cp -rf ${dir_src_kernel} ${newdir}
    echo "Replacing file names in diectory " ${newdir}
    pattern="*"${template_kernel}"*"
    echo "Finding in " ${newdir} "for " ${pattern}
    find ${newdir} -type f -name ${pattern} | while read FILE ; do
	rename 's/harris/safe/g' ${FILE}
    done
    new_pattern="*"${new_kernel}"*"
    find ${newdir} -type f -name ${new_pattern} | while read FILEN ; do
	echo "Replacing strings in file " ${FILEN}
	rpl ${template_kernel} ${new_kernel} ${FILEN}  
    done
done 

#find . -type f -name ${template_kernel} | while read FILE ; do
    #newfile="$(echo ${FILE} |sed -e 's/\\harris/safe/')" ;
    #find . -type f -exec rename -n 's/'${template_kernel}'/'${new_kernel}'/g' {} +
    #rename -n 's/harris/safe/g' *.jpg
    #echo "from" ${FILE} "to" ${newfile}
    #mv "${FILE}" "${newfile}" ;