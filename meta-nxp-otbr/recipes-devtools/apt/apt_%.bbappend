FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

customize_apt_conf_sample_matter() {
	cat > ${D}${sysconfdir}/apt/apt.conf.sample << EOF
Dir "${STAGING_DIR_NATIVE}/"
{
   State "var/lib/apt/"
   {
      Lists "#APTCONF#/lists/";
      status "#ROOTFS#/var/lib/dpkg/status";
   };
   Cache "var/cache/apt/"
   {
      Archives "archives/";
      pkgcache "";
      srcpkgcache "";
   };
   Bin "${STAGING_BINDIR_NATIVE}/"
   {
      methods "${STAGING_LIBDIR}/apt/methods/";
      gzip "/bin/gzip";
      dpkg "dpkg";
      dpkg-source "dpkg-source";
      dpkg-buildpackage "dpkg-buildpackage";
      apt-get "apt-get";
      apt-cache "apt-cache";
   };
   Etc "#APTCONF#"
   {
      Preferences "preferences";
   };
   Log "var/log/apt";
};

APT
{
  Install-Recommends "true";
  Immediate-Configure "false";
  Architecture "i586";
  Get
  {
     Assume-Yes "true";
  };
};

Acquire
{
  AllowInsecureRepositories "true";
};

DPkg::Options {"--root=#ROOTFS#";"--admindir=#ROOTFS#/var/lib/dpkg";"--force-all";"--no-debsig"};
DPkg::Path "";
EOF
}

do_install:append:class-native() {
	customize_apt_conf_sample_matter
}

do_install:append:class-nativesdk() {
	customize_apt_conf_sample_matter
        rm -rf ${D}${localstatedir}/log
}
