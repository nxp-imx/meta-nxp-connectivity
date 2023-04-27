python (){
    import shutil
    import zipfile

    src_uri=""
    src_uri = d.getVar("SRC_URI")
    bb.note("SRC_URI: %s" % src_uri)
    src_rev=""
    src_rev = d.getVar("SRCREV")
    bb.note("SRCREV: %s" % src_rev)
    extra_oemake=""
    extra_oemake = d.getVar("EXTRA_OECMAKE")
    bb.note("EXTRA_OECMAKE: %s" % extra_oemake)
    
    recipe_dir = d.getVar('THISDIR') + "/files"
    for file in os.listdir(recipe_dir):
        bb.note("Found file: %s " % file)
        if file.endswith(".zip"):
            os.path.join(recipe_dir, file)
            break
    otpatches_zipfile = recipe_dir + "/" + file

    if file.endswith(".zip"):
        with zipfile.ZipFile(otpatches_zipfile) as zip_file:
            for member in sorted(zip_file.namelist()):
                filename = os.path.basename(member)
                # skip directories
                if not filename:
                    continue
                # copy file (taken from zipfile's extract)
                source = zip_file.open(member)
                target = open(os.path.join(recipe_dir, filename), "wb")
                with source, target:
                    shutil.copyfileobj(source, target)

    release_note_file = "ReleaseNote-OPENTHREAD.txt"
    release_note = open(os.path.join(recipe_dir, release_note_file), "r")
    lines = release_note.readlines()
    release_note.close()

    openthread_url=""
    openthread_revision=""
    found_trigger_pattern=0
    for line in lines:
        if "OPENTHREAD for HOST" in line:
            found_trigger_pattern=1
            bb.note("Found trigger...%s " % found_trigger_pattern)
            continue
        if line.startswith("URL:") == 1 and found_trigger_pattern==1:
            line = line.replace("\n", " ")
            line = line.replace(" ", "")
            line = line.replace("https", "gitsm")
            line = line.split(":", 1)
            openthread_url += line[1]
            openthread_url += ";branch=main"
            bb.note("openthread_url: %s" % openthread_url)
            continue
        if line.startswith("version:") == 1 and found_trigger_pattern==1:
            line = line.replace("\n", " ")
            line = line.replace(" ", "")
            line = line.split(":", 1)
            openthread_revision += line[1]
            bb.note("openthread_revision: %s" % openthread_revision)
            break
    d.setVar("SRC_URI", openthread_url)
    d.setVar("SRCREV", openthread_revision)

    recipe_dir = d.getVar('THISDIR') + "/files"
    for file in sorted(os.listdir(recipe_dir)):
        if file.endswith(".patch"):
            bb.note("Adding: %s" % file)
            src_uri += " file://" + file
    d.appendVar("SRC_URI", src_uri)

    matter_option_file = "meta_matter_OPENTHREAD.txt"
    matter_option = open(os.path.join(recipe_dir, matter_option_file), "r")
    lines = matter_option.readlines()
    matter_option.close()

    build_options="-GNinja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DOT_COMPILE_WARNING_AS_ERROR=OFF -DOT_PLATFORM=posix -DOT_POSIX_MAX_POWER_TABLE=ON"
    found_trigger_pattern=0
    for line in lines:
        bb.note("line: %s" % line)
        if "meta-matter OPENTHREAD build options" in line:
            found_trigger_pattern=1
            continue
        if line.startswith("-DO") == 1:
            line = line.replace("\n", " ")
            line = line.replace("-DOPENTHREAD_CONFIG_", "-DOT_")
            line = line.replace("-DOPENTHREAD_POSIX_CONFIG_", "-DOT_POSIX_")
            line = line.replace("_ENABLE=1", "=ON")
            line = line.replace("_ENABLE=0", "=OFF")
            line = line.replace("OPENTHREAD_CONFIG_LOG_OUTPUT_PLATFORM_DEFINED", "PLATFORM_DEFINED")
            line = line.replace("-DOPENTHREAD_PLATFORM_POSIX=1", "-DOT_PLATFORM=posix")
            if found_trigger_pattern==1 and len(line.strip()) > 0:
                line = line.strip()
                line = line.replace("\n", " ")
                line += " "
                build_options += line

    d.appendVar("EXTRA_OECMAKE", build_options)

    src_uri = d.getVar("SRC_URI")
    bb.note("SRC_URI: %s" % src_uri)
    src_rev=""
    src_rev = d.getVar("SRCREV")
    bb.note("SRCREV: %s" % src_rev)
    extra_oemake=""
    extra_oemake = d.getVar("EXTRA_OECMAKE")
    bb.note("EXTRA_OECMAKE: %s" % extra_oemake)
}

python clean_recipe_folder(){
    recipe_dir = d.getVar('THISDIR') + "/files"
    for file in sorted(os.listdir(recipe_dir)):
        if file == "options_cmake_patch_for_TCP_option.patch":
            continue
        if file.endswith(".patch"):
            os.remove(recipe_dir + "/" + file)
        if file.endswith(".txt"):
            bb.note("Removing: %s" % file)
            os.remove(recipe_dir + "/" + file)
}

python print_settings(){
    src_uri = d.getVar("SRC_URI")
    bb.note("SRC_URI: %s" % src_uri)
    src_rev=""
    src_rev = d.getVar("SRCREV")
    bb.note("SRCREV: %s" % src_rev)
    extra_oemake=""
    extra_oemake = d.getVar("EXTRA_OECMAKE")
    bb.note("EXTRA_OECMAKE: %s" % extra_oemake)
}

do_compile[prefuncs] += "print_settings"
do_compile[postfuncs] += "print_settings"
do_package_qa[postfuncs] += "clean_recipe_folder"
do_clean[postfuncs] += "clean_recipe_folder"
do_cleanall[postfuncs] += "clean_recipe_folder"
do_cleansstate[postfuncs] += "clean_recipe_folder"

inherit cmake
EXTRA_OECMAKE += "-GNinja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DOT_COMPILE_WARNING_AS_ERROR=OFF -DCMAKE_BUILD_TYPE=Release"
