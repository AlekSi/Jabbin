#!/bin/sh

# ivan: this updates joimpsi_revision file which leads to disabling
# voip functions. So, I'm disabling this
exit 0

revision_file="tools/yastuff/joimpsi_revision.h"

update_revision_file() {
cat > $revision_file <<EOF
static QString JOIMPSI_VERSION="`cat ../joimpsi_version`";
static const int JOIMPSI_REVISION=$1;
EOF
	# echo "static const int JOIMPSI_REVISION=$1;" > $revision_file
	echo "'$revision_file' successfully updated."
}

get_revision() {
	echo "Getting current revision information from '$1'..."
	revision=`LANG=C svn info $1 | grep Revision | awk {'print \$2;'}`
	if [ "x$revision" = "x" ]; then
		echo -e "\tFailed."
	else
		update_revision_file $revision
		exit
	fi
}

get_revision "."
get_revision "https://svn.yandex.ru/yapsi/trunk/"
update_revision_file "0"
