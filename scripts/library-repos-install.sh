#!/bin/bash
# Git KiCad library repos:
#
# The "install_prerequisites" step is the only "distro dependent" one.  Could modify
# that step for other linux distros.
# This script requires "git".  The package bzr-git is not up to the task.
# The first time you run with option --install-or-update that is the slowest, because
# git clone from github.com is slow.
# After that updates should run faster.

# There are two reasons why you might want to run this script:
#
# 1) You want to contribute to the KiCad library team maintained libraries and have yet to
#    discover or have chosen not to use the COW feature in the Github "Plugin Type".
#
# 2) You want to run with local pretty footprint libraries and not those remotely located
#    on https://github.com using Github plugin.  After running this script you should be able to
#      a)  $ cp ~/kicad_sources/library-repos/kicad-library/template/fp-lib-table.for-pretty ~/fp-lib-table
#    and then
#      b) set your environment variable KISYSMOD to "~/kicad_sources/library-repos".
#         Edit /etc/profile.d/kicad.sh, then reboot.
#
#    This will use the KiCad plugin against the *.pretty dirs in that base dir.


# Set where the library repos will go, use a full path
WORKING_TREES=~/kicad_sources


usage()
{
    echo ""
    echo " usage:"
    echo ""
    echo "./library-sources-install.sh <cmd>"
    echo "    where <cmd> is one of:"
    echo "      --install-or-update         (from github, the library sources.)"
    echo "      --remove-all-libraries      (remove all *.pretty from $WORKING_TREES/library-repos/. )"
    echo "      --install-prerequisites     (install command tools needed here, run once first.)"
    echo "      --remove-orphaned-libraries (remove local libraries which have been deleted or renamed at github.)"
    echo ""
    echo "example:"
    echo '    $ ./library-sources-install.sh --install-or-update'
}


install_prerequisites()
{
    # Find a package manager, PM
    PM=$( command -v yum || command -v apt-get )

    # assume all these Debian, Mint, Ubuntu systems have same prerequisites
    if [ "$(expr match "$PM" '.*\(apt-get\)')" == "apt-get" ]; then
        #echo "debian compatible system"
        sudo apt-get install \
            git \
            curl \
            sed

    # assume all yum systems have same prerequisites
    elif [ "$(expr match "$PM" '.*\(yum\)')" == "yum" ]; then
        #echo "red hat compatible system"
        # Note: if you find this list not to be accurate, please submit a patch:
        sudo yum install \
            git \
            curl \
            sed
    else
        echo
        echo "Incompatible System. Neither 'yum' nor 'apt-get' found. Not possible to continue."
        echo
        exit 1
    fi
}


rm_build_dir()
{
    local dir="$1"
    # this file is often created as root, so remove as root
    sudo rm "$dir/install_manifest.txt" 2> /dev/null
    rm -rf "$dir"
}


cmake_uninstall()
{
    # assume caller set the CWD, and is only telling us about it in $1
    local dir="$1"

    cwd=`pwd`
    if [ "$cwd" != "$dir" ]; then
        echo "missing dir $dir"
    elif [ ! -e install_manifest.txt  ]; then
        echo
        echo "Missing file $dir/install_manifest.txt."
    else
        echo "uninstalling from $dir"
        sudo make uninstall
        sudo rm install_manifest.txt
    fi
}


detect_pretty_repos()
{
    # Use github API to list repos for org KiCad, then subset the JSON reply for only
    # *.pretty repos
    PRETTY_REPOS=`curl https://api.github.com/orgs/KiCad/repos?per_page=2000 2> /dev/null \
        | grep full_name | grep pretty \
        | sed -r  's:.+ "KiCad/(.+)",:\1:'`

    #echo "PRETTY_REPOS:$PRETTY_REPOS"
}


checkout_or_update_libraries()
{
    if [ ! -d "$WORKING_TREES" ]; then
        sudo mkdir -p "$WORKING_TREES"
        echo " mark $WORKING_TREES as owned by me"
        sudo chown -R `whoami` "$WORKING_TREES"
    fi
    cd $WORKING_TREES

    detect_pretty_repos

    if [ ! -e "$WORKING_TREES/library-repos" ]; then
        mkdir -p "$WORKING_TREES/library-repos"
    fi

    for repo in kicad-library $PRETTY_REPOS; do
        # echo "repo:$repo"

        if [ ! -e "$WORKING_TREES/library-repos/$repo" ]; then

            # Be _sure_ and preserve the directory name, we want extension .pretty not .pretty.git.
            # That way those repos can serve as pretty libraries directly if need be.

            echo "installing $WORKING_TREES/library-repos/$repo"
            git clone "https://github.com/KiCad/$repo" "$WORKING_TREES/library-repos/$repo"
        else
            echo "updating $WORKING_TREES/library-repos/$repo"
            cd "$WORKING_TREES/library-repos/$repo"
            git pull
        fi
    done
}


listcontains()
{
    local list=$1
    local item=$2
    local ret=1
    local OIFS=$IFS

    # omit the space character from internal field separator.
    IFS=$'\n'

    for word in $list; do
        if [ "$word" == "$item" ]; then
            ret=0
            break
        fi
    done

    IFS=$OIFS
    return $ret
}


remove_orphaned_libraries()
{
    cd $WORKING_TREES/library-repos

    if [ $? -ne 0 ]; then
        echo "Directory $WORKING_TREES/library-repos does not exist."
        echo "The option --remove-orphaned-libraries should be used only after you've run"
        echo "the --install-or-update at least once."
        exit 2
    fi

    detect_pretty_repos

    for mylib in *.pretty; do
        echo "checking local lib: $mylib"

        if ! listcontains "$PRETTY_REPOS" "$mylib"; then
            echo "Removing orphaned local library $WORKING_TREES/library-repos/$mylib"
            rm -rf "$mylib"
        fi
    done
}


if [ $# -eq 1 -a "$1" == "--install-or-update" ]; then
    checkout_or_update_libraries
    exit
fi


if [ $# -eq 1 -a "$1" == "--remove-orphaned-libraries" ]; then
    remove_orphaned_libraries
    exit
fi


if [ $# -eq 1 -a "$1" == "--remove-all-libraries" ]; then
    rm -rf "$WORKING_TREES/library-repos"
    exit
fi


if [ $# -eq 1 -a "$1" == "--install-prerequisites" ]; then
    install_prerequisites
    exit
fi


usage
