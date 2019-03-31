#ifndef UTILS_HH
#define UTILS_HH

#include <string>

extern "C" {
#include <emu_c_utils/layout.h>
}

void print_emu_ptr(std::string name, void * r)
{
    emu_pointer pchk = examine_emu_pointer(r);

    if (pchk.view == 2)
    {
        printf("emu ptr: %s view: %ld\n", name.c_str(), pchk.view);
    }
    else
    {
        printf("emu ptr: %s view: %ld, node_id: %ld, nodelet_id: %ld, "
               "nodelet_addr: %ld, byte_offset: %ld\n",
               name.c_str(),
               pchk.view, pchk.node_id, pchk.nodelet_id, pchk.nodelet_addr,
               pchk.byte_offset);
    }
}

#endif // UTILS_HH
