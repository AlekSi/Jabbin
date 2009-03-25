#!/bin/sh

# ivan: this updates jabbinpsi_revision file which leads to disabling
# voip functions. So, I'm disabling this
exit 0

revision_file="tools/yastuff/jabbinpsi_revision.h"

update_revision_file() {
cat > $revision_file <<EOF
static QString JABBINPSI_VERSION="`cat ../jabbinpsi_version`";
static const int JABBINPSI_REVISION=$1;
EOF
	# echo "static const int JABBINPSI_REVISION=$1;" > $revision_file
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
