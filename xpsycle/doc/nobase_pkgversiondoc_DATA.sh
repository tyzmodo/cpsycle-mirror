#! /bin/sh

main()
{
	cd $(dirname $0) &&
	stanza=$(basename $0 .sh) &&
	out=$stanza.am &&
	echo "# this file was autogenerated by $0 ; do not edit" > $out &&
	for x in $(find ./ -follow \( -type d \( -name todo -or -name .svn \) -prune \) -or \( \( -name \*.txt -or -name \*.html -or -name \*.pdf -or -name \*.png -or -name \*.jpeg -or -name \*.jpg -or -name \*.xml -or -name \*.xml.in \) -print \))
	do
		case $x in
			*.in)
				x=$(dirname $x)/$(basename $x .in)
			;;
		esac &&
		echo $stanza += $x >> $out ||
		return
	done
} &&

main "$0"
