#!/bin/sh

perl -pi -e 'BEGIN { chomp($date=`date -R`); $set = 0; }
	s/(\d+)\.(\d+)/sprintf "$1.%d",$2 + 1/e unless $set;
	s/>.*/>  $date/, $set = 1 if (/--.*>/ && !$set);' debian/changelog
