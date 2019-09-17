DESCRIPTION = "Demo scripts and data files for A5" 
SECTION = "extras" 
LICENSE = "CLOSED" 
PR = "1.0" 
 
RDEPENDS_${PN} = "bash"

do_install() { 
 install -d ${D}${bindir} 
 
 scripts_files=$(ls ${ST_LOCAL_TOOLS}stm-trace/) 
 for file in $scripts_files; do  
 install -m 0777 ${ST_LOCAL_TOOLS}stm-trace/$file ${D}${bindir} 
 done
 
} 
 
ALLOW_EMPTY_${PN}="1"
