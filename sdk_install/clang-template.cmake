# compilation target
--target=@TEMPLATE_TARGET@
-D_GNU_SOURCE=1
-D__GNU__
-D__rtems__

# compiler flags
-fPIE

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
-lm 
-lc 
-lg 
-ldandelion_file_system
-ldandelion_runtime 
-ldandelion_system