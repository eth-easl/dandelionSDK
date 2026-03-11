#include <errno.h>
#include <resolv.h>

int dn_expand(const unsigned char *msg, const unsigned char *eomorig,
              const unsigned char *comp_dn, char *exp_dn, int length) {
  (void)msg;
  (void)eomorig;
  (void)comp_dn;
  (void)exp_dn;
  (void)length;
  errno = ENOSYS;
  return -1;
}
