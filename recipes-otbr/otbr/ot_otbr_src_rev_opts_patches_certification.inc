python (){
    import shutil
    import zipfile

    src_uri=""
    src_rev=""
    extra_oemake=""

    recipe_dir = d.getVar('THISDIR') + "/files"
    for file in os.listdir(recipe_dir):
        #bb.note("Found file: %s " % file)
        if file.endswith(".zip"):
            os.path.join(recipe_dir, file)
            otbrpatches_zipfile = recipe_dir + "/" + file
            archive = zipfile.ZipFile(otbrpatches_zipfile)
            archive.extractall(recipe_dir)
            root_path_in_zip, = zipfile.Path(archive).iterdir()
            break

    release_note_file = root_path_in_zip.name + "/ReleaseNote-OTBR.txt"
    release_note = open(os.path.join(recipe_dir, release_note_file), "r")
    lines = release_note.readlines()
    release_note.close()

    otbr_url=""
    otbr_revision=""
    openthread_for_otbr_revision=""
    found_otbr_trigger_pattern=0
    found_openthread_trigger_pattern=0
    for line in lines:
        if "OT-BR" in line:
            found_otbr_trigger_pattern=1
            #bb.note("Found OT-BR trigger...%s " % found_otbr_trigger_pattern)
            continue
        if line.startswith("URL:") == 1 and found_otbr_trigger_pattern==1:
            line = line.replace("\n", " ");line = line.replace(" ", "");line = line.replace("https", "gitsm");line = line.split(":", 1)
            otbr_url += line[1];otbr_url += ";branch=main"
            #bb.note("otbr_url: %s" % otbr_url)
            continue
        if line.startswith("version:") == 1 and found_otbr_trigger_pattern==1:
            line = line.replace("\n", " ");line = line.replace(" ", "");line = line.split(":", 1)
            otbr_revision += line[1]
            #bb.note("otbr_revision: %s" % otbr_revision)
            found_otbr_trigger_pattern=0
            continue
        if "OPENTHREAD for OTBR" in line:
            found_openthread_trigger_pattern=1
            #bb.note("Found OPENTHREAD for OTBR trigger...%s " % found_openthread_trigger_pattern)
            continue
        if line.startswith("version:") == 1 and found_openthread_trigger_pattern==1:
            line = line.replace("\n", " ");line = line.replace(" ", "");line = line.split(":", 1)
            openthread_for_otbr_revision += line[1]
            #bb.note("openthread_for_otbr_revision: %s" % openthread_for_otbr_revision)
            found_openthread_trigger_pattern=0
            break
    d.setVar("SRC_URI", otbr_url)
    d.setVar("SRCREV", otbr_revision)
    d.setVar("SHA1_OPENTHREAD_FOR_OTBR", openthread_for_otbr_revision)
    d.setVar("OPENTHREAD_FOR_OTBR_PATCHES_PATH", openthread_for_otbr_revision)

    recipe_dir = d.getVar('THISDIR') + "/files/" + root_path_in_zip.name
    for file in sorted(os.listdir(recipe_dir)):
        if file.endswith(".patch"):
            if d.getVar('OT_RCP_BUS') in file:
                #bb.note("Adding: %s" % file)
                src_uri += " file://" + root_path_in_zip.name + "/" + file
    d.appendVar("SRC_URI", src_uri)

    #bb.note("recipe_dir: %s" % recipe_dir)
    d.setVar("OPENTHREAD_FOR_OTBR_PATCHES_PATH", recipe_dir)

    recipe_dir = d.getVar('THISDIR') + "/files"
    matter_option_file = root_path_in_zip.name + "/meta_matter_OTBR.txt"
    matter_option = open(os.path.join(recipe_dir, matter_option_file), "r")
    lines = matter_option.readlines()
    matter_option.close()

    build_options="-GNinja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DOT_COMPILE_WARNING_AS_ERROR=OFF -DBUILD_TESTING=OFF "
    found_trigger_pattern=0
    for line in lines:
        if "meta-matter OTBR build options" in line:
            found_trigger_pattern=1
            continue
        if line.startswith("-DO") == 1:
            line = line.replace("\n", " ");line = line.replace("-DOPENTHREAD_CONFIG_", "-DOT_");line = line.replace("-DOPENTHREAD_POSIX_CONFIG_", "-DOT_POSIX_")
            line = line.replace("_ENABLE=1", "=ON");line = line.replace("_ENABLE=0", "=OFF")
            line = line.replace("OPENTHREAD_CONFIG_LOG_OUTPUT_PLATFORM_DEFINED", "PLATFORM_DEFINED")
            line = line.replace("-DOPENTHREAD_PLATFORM_POSIX=1", "-DOT_PLATFORM=posix")
            if found_trigger_pattern==1 and len(line.strip()) > 0:
                line = line.strip()
                line = line.replace("\n", " ")
                line += " "
                build_options += line

    d.appendVar("EXTRA_OECMAKE", build_options)

    src_uri = d.getVar("SRC_URI")
    #bb.note("SRC_URI: %s" % src_uri)
    src_rev=""
    src_rev = d.getVar("SRCREV")
    #bb.note("SRCREV: %s" % src_rev)
    sha1_openthread = d.getVar("SHA1_OPENTHREAD_FOR_OTBR")
    #bb.note("SHA1_OPENTHREAD_FOR_OTBR: %s" % sha1_openthread)
    openthread_for_otbr_patches_path = d.getVar("OPENTHREAD_FOR_OTBR_PATCHES_PATH")
    #bb.note("OPENTHREAD_FOR_OTBR_PATCHES_PATH: %s" % openthread_for_otbr_patches_path)
    extra_oemake=""
    extra_oemake = d.getVar("EXTRA_OECMAKE")
    #bb.note("EXTRA_OECMAKE: %s" % extra_oemake)
}

addtask do_openthread_repo_reset_and_patch after do_unpack before do_patch
do_openthread_repo_reset_and_patch() {
    cd ${S}/third_party/openthread/repo
    echo "Applying on ${S}/third_party/openthread/repo:"
    echo "-> git reset --hard ${SHA1_OPENTHREAD_FOR_OTBR}"
    git reset --hard ${SHA1_OPENTHREAD_FOR_OTBR}
    result=$?
    if [ ${result} -ne 0 ]; then
        echo "Cannot reset openthread to version '$SHA1_OPENTHREAD_FOR_OTBR', ABORT"
        exit
    fi

    mkdir ${WORKDIR}/tmp_patches
    cp ${OPENTHREAD_FOR_OTBR_PATCHES_PATH}/openthread_patches/* ${WORKDIR}/tmp_patches
    PATCHES_FILES_LIST=$(ls ${WORKDIR}/tmp_patches)
    for i in ${PATCHES_FILES_LIST}; do
        true | git apply ${WORKDIR}/tmp_patches/${i} --check
        if [ $? -eq 0 ]; then
            echo "----> Applying ${i}"
            git apply ${WORKDIR}/tmp_patches/${i}
        fi
    done
    rm -rf  ${WORKDIR}/tmp_patches
}

do_install:append() {
    mkdir ${D}${sysconfdir}/init.d
    cp ${S}/script/otbr-firewall ${D}${sysconfdir}/init.d
    chmod +x ${D}${sysconfdir}/init.d/otbr-firewall
    cp ${S}/script/README_firewall.txt ${D}${sysconfdir}/init.d
}
