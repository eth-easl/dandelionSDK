# compilation target
--target=@TEMPLATE_TARGET@
-D_GNU_SOURCE=1
-D__GNU__
-D__rtems__

# compiler flags
-fPIE

# include flags
-nostdinc
-idirafter<CFGDIR>/include
-idirafter<CFGDIR>/include/sys

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