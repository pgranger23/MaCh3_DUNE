#!/bin/bash
#
# A script to link required files from iRODS at the proper place
# to copy in the main MaCh3 dir : GlobalAnalysisTools/GlobalFitters/MaCh3
# 12/10/2015

MACH3DIR=`pwd`
#FILESDIR=/vols/t2k/users/pjd12/data
FILESDIR=/vols/t2k/users/ljw20/data/DUNE_2021

#copy DUNE splines
 if [ ! -d "$MACH3DIR/inputs/DUNE_CAF_files_2021/" ]
  then
    mkdir $MACH3DIR/inputs/DUNE_CAF_files_2021
 fi

 ln -sf ${FILESDIR}/DUNE_2021_CAFs/*root inputs/DUNE_CAF_files_2021


 if [ ! -d "$MACH3DIR/inputs/DUNE_spline_files_2021/" ]
  then
    mkdir $MACH3DIR/inputs/DUNE_spline_files_2021
 fi

 ln -sf ${FILESDIR}/DUNE_2021_splines/*root inputs/DUNE_spline_files_2021
