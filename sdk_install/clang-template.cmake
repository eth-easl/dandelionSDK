# compilation target
--target=@TEMPLATE_TARGET@
# -D_GNU_SOURCE=1
# -D__GNU__
# -D__rtems__
# prevent thread local storage in compilation, TODO: remove if we can provide TLS
-D__thread=""
-D__DANDELION__

# compiler flags
-fPIE
#-march=haswell
#-mtune=haswell

# include flags
-nostdinc
-isystem<CFGDIR>/include
-isystem<CFGDIR>/include/sys
-isystemCOMPILER_INCLUDES

# linker flags
-T<CFGDIR>/linker.ld
-fuse-ld=lld
-static
-nostdlib
-L<CFGDIR>/lib
-L<CFGDIR>/lib/generic
-lm 
-lc 
-lg 
-lc_extension
-ldandelion_file_system
-ldandelion_runtime 
-ldandelion_system
-lclang_rt.builtins-@ARCHITECTURE@
