/*********************IMPORTANT DRAKVUF LICENSE TERMS***********************
 *                                                                         *
 * DRAKVUF (C) 2014-2022 Tamas K Lengyel.                                  *
 * Tamas K Lengyel is hereinafter referred to as the author.               *
 * This program is free software; you may redistribute and/or modify it    *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; Version 2 ("GPL"), BUT ONLY WITH ALL OF THE   *
 * CLARIFICATIONS AND EXCEPTIONS DESCRIBED HEREIN.  This guarantees your   *
 * right to use, modify, and redistribute this software under certain      *
 * conditions.  If you wish to embed DRAKVUF technology into proprietary   *
 * software, alternative licenses can be acquired from the author.         *
 *                                                                         *
 * Note that the GPL places important restrictions on "derivative works",  *
 * yet it does not provide a detailed definition of that term.  To avoid   *
 * misunderstandings, we interpret that term as broadly as copyright law   *
 * allows.  For example, we consider an application to constitute a        *
 * derivative work for the purpose of this license if it does any of the   *
 * following with any software or content covered by this license          *
 * ("Covered Software"):                                                   *
 *                                                                         *
 * o Integrates source code from Covered Software.                         *
 *                                                                         *
 * o Reads or includes copyrighted data files.                             *
 *                                                                         *
 * o Is designed specifically to execute Covered Software and parse the    *
 * results (as opposed to typical shell or execution-menu apps, which will *
 * execute anything you tell them to).                                     *
 *                                                                         *
 * o Includes Covered Software in a proprietary executable installer.  The *
 * installers produced by InstallShield are an example of this.  Including *
 * DRAKVUF with other software in compressed or archival form does not     *
 * trigger this provision, provided appropriate open source decompression  *
 * or de-archiving software is widely available for no charge.  For the    *
 * purposes of this license, an installer is considered to include Covered *
 * Software even if it actually retrieves a copy of Covered Software from  *
 * another source during runtime (such as by downloading it from the       *
 * Internet).                                                              *
 *                                                                         *
 * o Links (statically or dynamically) to a library which does any of the  *
 * above.                                                                  *
 *                                                                         *
 * o Executes a helper program, module, or script to do any of the above.  *
 *                                                                         *
 * This list is not exclusive, but is meant to clarify our interpretation  *
 * of derived works with some common examples.  Other people may interpret *
 * the plain GPL differently, so we consider this a special exception to   *
 * the GPL that we apply to Covered Software.  Works which meet any of     *
 * these conditions must conform to all of the terms of this license,      *
 * particularly including the GPL Section 3 requirements of providing      *
 * source code and allowing free redistribution of the work as a whole.    *
 *                                                                         *
 * Any redistribution of Covered Software, including any derived works,    *
 * must obey and carry forward all of the terms of this license, including *
 * obeying all GPL rules and restrictions.  For example, source code of    *
 * the whole work must be provided and free redistribution must be         *
 * allowed.  All GPL references to "this License", are to be treated as    *
 * including the terms and conditions of this license text as well.        *
 *                                                                         *
 * Because this license imposes special exceptions to the GPL, Covered     *
 * Work may not be combined (even as part of a larger work) with plain GPL *
 * software.  The terms, conditions, and exceptions of this license must   *
 * be included as well.  This license is incompatible with some other open *
 * source licenses as well.  In some cases we can relicense portions of    *
 * DRAKVUF or grant special permissions to use it in other open source     *
 * software.  Please contact tamas.k.lengyel@gmail.com with any such       *
 * requests.  Similarly, we don't incorporate incompatible open source     *
 * software into Covered Software without special permission from the      *
 * copyright holders.                                                      *
 *                                                                         *
 * If you have any questions about the licensing restrictions on using     *
 * DRAKVUF in other works, are happy to help.  As mentioned above,         *
 * alternative license can be requested from the author to integrate       *
 * DRAKVUF into proprietary applications and appliances.  Please email     *
 * tamas.k.lengyel@gmail.com for further information.                      *
 *                                                                         *
 * If you have received a written license agreement or contract for        *
 * Covered Software stating terms other than these, you may choose to use  *
 * and redistribute Covered Software under those terms instead of these.   *
 *                                                                         *
 * Source is provided to this software because we believe users have a     *
 * right to know exactly what a program is going to do before they run it. *
 * This also allows you to audit the software for security holes.          *
 *                                                                         *
 * Source code also allows you to port DRAKVUF to new platforms, fix bugs, *
 * and add new features.  You are highly encouraged to submit your changes *
 * on https://github.com/tklengyel/drakvuf, or by other methods.           *
 * By sending these changes, it is understood (unless you specify          *
 * otherwise) that you are offering unlimited, non-exclusive right to      *
 * reuse, modify, and relicense the code.  DRAKVUF will always be          *
 * available Open Source, but this is important because the inability to   *
 * relicense code has caused devastating problems for other Free Software  *
 * projects (such as KDE and NASM).                                        *
 * To specify special license conditions of your contributions, just say   *
 * so when you send them.                                                  *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the DRAKVUF   *
 * license file for more details (it's in a COPYING file included with     *
 * DRAKVUF, and also available from                                        *
 * https://github.com/tklengyel/drakvuf/COPYING)                           *
 *                                                                         *
 ***************************************************************************/

#include <libvmi/libvmi.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <stdio.h>
#include <glib.h>
#include <limits.h>

#include "private.h"
#include "win-offsets.h"
#include "win-wow-offsets.h"
#include "win-error-codes.h"
#include "win.h"

#define MMVAD_MAX_DEPTH (100)

#define POOL_TAG_SIZE_X86 0x4
#define POOL_TAG_SIZE_X64 0xC

#define POOL_TAG_VAD    (0x20646156) // daV
#define POOL_TAG_VADL   (0x6c646156) // ldaV
#define POOL_TAG_VADM   (0x6d646156) // mdaV

#define RTL_BALANCED_NODE_RESERVED_PARENT_MASK 3
#define RTL_BALANCED_NODE_GET_PARENT_POINTER(Node) \
     ((PRTL_BALANCED_NODE)((Node)->ParentValue & \
                           ~RTL_BALANCED_NODE_RESERVED_PARENT_MASK))

typedef enum dispatcher_object
{
    __DISPATCHER_INVALID_OBJECT = 0,
    DISPATCHER_PROCESS_OBJECT = 3,
    DISPATCHER_THREAD_OBJECT  = 6
} dispatcher_object_t ;

bool win_search_modules( drakvuf_t drakvuf, const char* module_name, bool (*visitor_func)(drakvuf_t drakvuf, const module_info_t* module_info, void* visitor_ctx), void* visitor_ctx, addr_t eprocess_addr, addr_t wow_process, vmi_pid_t pid, access_context_t* ctx );
bool win_search_modules_wow( drakvuf_t drakvuf, const char* module_name, bool (*visitor_func)(drakvuf_t drakvuf, const module_info_t* module_info, void* visitor_ctx), void* visitor_ctx, addr_t eprocess_addr, addr_t wow_peb, vmi_pid_t pid, access_context_t* ctx );

static bool win_get_current_kpcr(drakvuf_t drakvuf, drakvuf_trap_info_t* info, addr_t* kpcr, addr_t* prcb)
{
    vmi_instance_t vmi = drakvuf->vmi;
    // TODO: check whether we could use ss_arbytes here
    unsigned int cpl = info->regs->cs_sel & 3;

    if (VMI_PM_IA32E == drakvuf->pm)
    {
        *prcb = drakvuf->offsets[KPCR_PRCB];
        *kpcr = cpl ? info->regs->shadow_gs :
            /* if we are in kernel mode we might trap before swapgs */
            VMI_GET_BIT(info->regs->gs_base, 47) ? info->regs->gs_base : info->regs->shadow_gs;
    }
    else
    {
        /*
         * "In 32-bit Windows, entering kernel mode gets fs loaded with a GDT selector (0x0030)
         * for a segment whose base address is that of the processor’s KPCR."
         * https://www.geoffchappell.com/studies/windows/km/ntoskrnl/structs/kpcr.htm
         * https://wiki.osdev.org/Global_Descriptor_Table
         */
        if ( cpl )
        {
            uint16_t fs_low = 0;
            uint8_t fs_mid = 0, fs_high = 0;

            addr_t gdt = info->regs->gdtr_base;

            if (VMI_FAILURE == vmi_read_16_va(vmi, gdt + 0x32, 0, &fs_low))
                return false;
            if (VMI_FAILURE == vmi_read_8_va(vmi, gdt + 0x34, 0, &fs_mid))
                return false;
            if (VMI_FAILURE == vmi_read_8_va(vmi, gdt + 0x37, 0, &fs_high))
                return false;

            *kpcr = ((uint32_t)fs_low) | ((uint32_t)fs_mid) << 16 | ((uint32_t)fs_high) << 24;
        }
        else
            *kpcr = info->regs->fs_base;

        *prcb = drakvuf->offsets[KPCR_PRCBDATA];
    }

    return true;
}

bool win_get_current_irql(drakvuf_t drakvuf, drakvuf_trap_info_t* info, uint8_t* irql)
{
    vmi_instance_t vmi = drakvuf->vmi;
    addr_t prcb = 0;
    addr_t kpcr = 0;

    if ( !win_get_current_kpcr(drakvuf, info, &kpcr, &prcb) ||
        (VMI_SUCCESS != vmi_read_8_va(vmi, kpcr + drakvuf->offsets[KPCR_IRQL], 0, irql)) )
        return false;

    return true;
}

addr_t win_get_current_thread(drakvuf_t drakvuf, drakvuf_trap_info_t* info)
{
    vmi_instance_t vmi = drakvuf->vmi;
    addr_t thread = 0;
    addr_t prcb = 0;
    addr_t kpcr = 0;

    if (! win_get_current_kpcr(drakvuf, info, &kpcr, &prcb) )
    {
        return 0;
    }

    if (VMI_SUCCESS != vmi_read_addr_va(vmi, kpcr + prcb + drakvuf->offsets[KPRCB_CURRENTTHREAD], 0, &thread))
    {
        return 0;
    }

    return thread;
}

addr_t win_get_current_thread_teb(drakvuf_t drakvuf, drakvuf_trap_info_t* info)
{
    vmi_instance_t vmi = drakvuf->vmi;

    addr_t kthread = win_get_current_thread(drakvuf, info);
    if (!kthread)
        return 0;

    addr_t teb = 0;
    if (VMI_SUCCESS != vmi_read_addr_va(vmi, kthread + drakvuf->offsets[KTHREAD_TEB], 0, &teb))
        return 0;

    return teb;
}

addr_t win_get_current_thread_stackbase(drakvuf_t drakvuf, drakvuf_trap_info_t* info)
{
    vmi_instance_t vmi = drakvuf->vmi;

    addr_t kthread = win_get_current_thread(drakvuf, info);
    if (!kthread)
        return 0;

    addr_t stackbase = 0;
    if (VMI_SUCCESS != vmi_read_addr_va(vmi, kthread + drakvuf->offsets[KTHREAD_STACKBASE], 0, &stackbase))
        return 0;

    return stackbase;
}

addr_t win_get_current_process(drakvuf_t drakvuf, drakvuf_trap_info_t* info)
{
    addr_t process;

    addr_t thread = win_get_current_thread(drakvuf, info);

    if (thread == 0 || VMI_SUCCESS != vmi_read_addr_va(drakvuf->vmi, thread + drakvuf->offsets[KTHREAD_PROCESS], 0, &process))
    {
        return 0;
    }

    return process;
}

addr_t win_get_current_attached_process(drakvuf_t drakvuf, drakvuf_trap_info_t* info)
{
    uint8_t apcstateindex;
    addr_t process;
    addr_t thread = win_get_current_thread(drakvuf, info);
    if (thread &&
        VMI_SUCCESS == vmi_read_8_va(drakvuf->vmi, thread + drakvuf->offsets[KTHREAD_APCSTATEINDEX], 0, &apcstateindex) &&
        apcstateindex &&
        VMI_SUCCESS == vmi_read_addr_va(drakvuf->vmi, thread + drakvuf->offsets[KTHREAD_APCSTATE] + drakvuf->offsets[KAPC_STATE_PROCESS], 0, &process))
    {
        return process;
    }

    return win_get_current_process(drakvuf, info);
}

bool win_get_last_error(drakvuf_t drakvuf, drakvuf_trap_info_t* info, uint32_t* err, const char** err_str)
{
    if (!err || !err_str)
        return false;

    vmi_instance_t vmi = drakvuf->vmi;

    addr_t eprocess = win_get_current_process(drakvuf, info);
    addr_t cr3 = 0;
    vmi_pid_t pid = 0;
    if (eprocess && win_get_process_pid(drakvuf, eprocess, &pid))
        if (VMI_SUCCESS != vmi_pid_to_dtb(vmi, pid, &cr3))
            return false;

    addr_t teb = win_get_current_thread_teb(drakvuf, info);
    if (!teb)
        return false;

    ACCESS_CONTEXT(ctx,
        .translate_mechanism = VMI_TM_PROCESS_DTB,
        .dtb = cr3,
        .addr = teb + drakvuf->offsets[TEB_LASTERRORVALUE]
    );

    if (VMI_SUCCESS != vmi_read_32(vmi, &ctx, err))
        return false;

    if (*err >= __WIN_ERROR_CODES_MAX)
        return false;

    if (win_error_code_names[*err])
        *err_str = win_error_code_names[*err];

    return true;
}

static addr_t win_get_file_name_ptr_from_controlarea(drakvuf_t drakvuf, addr_t control_area)
{
    bool is32bit = (drakvuf_get_page_mode(drakvuf) != VMI_PM_IA32E);
    addr_t ex_fast_ref_mask = is32bit ? ~7ULL : ~0xfULL;

    ACCESS_CONTEXT(ctx,
        .translate_mechanism = VMI_TM_PROCESS_PID,
        .pid = 4
    );

    ctx.addr = control_area + drakvuf->offsets[CONTROL_AREA_FILEPOINTER];

    addr_t file_object = 0;
    if (vmi_read_addr(drakvuf->vmi, &ctx, &file_object) == VMI_SUCCESS)
    {
        // file_object is a special _EX_FAST_REF pointer,
        // we need to explicitly clear low bits
        file_object &= ex_fast_ref_mask;

        if (file_object)
            return file_object + drakvuf->offsets[FILEOBJECT_NAME];
    }

    return 0;
}

static unicode_string_t* win_get_process_full_name(drakvuf_t drakvuf, addr_t eprocess_base)
{
    addr_t image_file_name_addr = 0;
    if ( vmi_read_addr_va(drakvuf->vmi,
            eprocess_base + drakvuf->offsets[EPROCESS_PROCCREATIONINFO] + drakvuf->offsets[PROCCREATIONINFO_IMAGEFILENAME],
            0, &image_file_name_addr) != VMI_SUCCESS )
    {
        PRINT_DEBUG("in win_get_process_full_name(...) couldn't read IMAGEFILENAME address\n");
        return NULL;
    }

    if (image_file_name_addr)
        return drakvuf_read_unicode_va(drakvuf,
                image_file_name_addr + drakvuf->offsets[OBJECTNAMEINFORMATION_NAME], 0);
    else
    {
        addr_t section_object = 0;
        if ( vmi_read_addr_va(drakvuf->vmi,
                eprocess_base + drakvuf->offsets[EPROCESS_SECTIONOBJECT],
                0, &image_file_name_addr) == VMI_SUCCESS)
        {
            addr_t control_area = 0;
            bool is_win7_win8 = vmi_get_winver( drakvuf->vmi ) <= VMI_OS_WINDOWS_8;
            if (is_win7_win8)
            {
                addr_t segment = 0;
                if ( vmi_read_addr_va(drakvuf->vmi,
                        section_object + drakvuf->offsets[SECTIONOBJECT_SEGMENT],
                        0, &segment) == VMI_SUCCESS)
                {
                    vmi_read_addr_va(drakvuf->vmi,
                        segment + drakvuf->offsets[SEGMENT_CONTROLAREA],
                        0, &control_area);
                }
            }
            else
            {
                vmi_read_addr_va(drakvuf->vmi,
                    section_object + drakvuf->offsets[SECTION_CONTROLAREA], 0, &control_area);
            }

            if (control_area)
                return drakvuf_read_unicode_va(drakvuf, win_get_file_name_ptr_from_controlarea(drakvuf, control_area), 0);
        }
    }

    return NULL;
}

static char* win_get_process_name_impl(unicode_string_t* fullname, bool fullpath)
{
    if ( !(fullname && fullname->contents && strlen((const char*)fullname->contents) > 0) )
        return NULL;

    // Moving ownership of fullname->contents to name for later cleanup
    char* name = g_strdup((char*)fullname->contents);
    // ImageFileName size differs between kernel builds & versions,
    // relying on it would sometimes yield incomplete process name.
    if ( !fullpath )
    {
        char** tokens = g_strsplit(name, "\\", -1);
        int index = 0;

        if (tokens && tokens[index])
        {
            g_free(name);
            while (tokens[index])
                name = tokens[index++];
            name = g_strdup(name);
        }

        g_strfreev(tokens);
    }

    return name;
}

char* win_get_process_name(drakvuf_t drakvuf, addr_t eprocess_base, bool fullpath)
{
    unicode_string_t* fullname = win_get_process_full_name(drakvuf, eprocess_base);
    char* name = win_get_process_name_impl(fullname, fullpath);
    vmi_free_unicode_str(fullname);

    if (!name)
        name = vmi_read_str_va(drakvuf->vmi, eprocess_base + drakvuf->offsets[EPROCESS_PNAME], 0);

    return name;
}

char* win_get_process_commandline(drakvuf_t drakvuf, drakvuf_trap_info_t* info, addr_t eprocess_base)
{
    vmi_instance_t vmi = drakvuf->vmi;

    ACCESS_CONTEXT(ctx,
        .translate_mechanism = VMI_TM_PROCESS_DTB,
        .dtb = info->regs->cr3
    );

    addr_t peb = 0;
    ctx.addr = eprocess_base + drakvuf->offsets[EPROCESS_PEB];
    if (VMI_SUCCESS != vmi_read_addr(vmi, &ctx, &peb))
        return NULL;

    addr_t proc_params = 0;
    ctx.addr = peb + drakvuf->offsets[PEB_PROCESSPARAMETERS];
    if (VMI_SUCCESS != vmi_read_addr(vmi, &ctx, &proc_params))
        return NULL;

    addr_t cmdline_va = proc_params + drakvuf->offsets[RTL_USER_PROCESS_PARAMETERS_COMMANDLINE];

    unicode_string_t* cmdline_us = drakvuf_read_unicode(drakvuf, info, cmdline_va);
    if (!cmdline_us)
        return NULL;

    char* cmdline = (char*)cmdline_us->contents;
    g_free( (gpointer)cmdline_us );

    return cmdline;
}

bool win_get_process_pid(drakvuf_t drakvuf, addr_t eprocess_base, vmi_pid_t* pid)
{
    if ( VMI_SUCCESS == vmi_read_32_va(drakvuf->vmi, eprocess_base + drakvuf->offsets[EPROCESS_PID], 0, (uint32_t*)pid) )
        return true;

    return false;
}

char* win_get_current_process_name(drakvuf_t drakvuf, drakvuf_trap_info_t* info, bool fullpath)
{
    return win_get_process_name(drakvuf, win_get_current_process(drakvuf, info), fullpath);
}

int64_t win_get_process_userid(drakvuf_t drakvuf, addr_t eprocess_base)
{
    addr_t peb;
    addr_t userid;
    vmi_instance_t vmi = drakvuf->vmi;
    ACCESS_CONTEXT(ctx,
        .translate_mechanism = VMI_TM_PROCESS_DTB
    );

    if (!eprocess_base)
        return -1;

    if (VMI_FAILURE == vmi_read_addr_va(vmi, eprocess_base + drakvuf->offsets[EPROCESS_PEB], 0, &peb))
        return -1;

    if (VMI_FAILURE == vmi_read_addr_va(vmi, eprocess_base + drakvuf->offsets[EPROCESS_PDBASE], 0, &ctx.dtb))
        return -1;

    ctx.addr = peb + drakvuf->offsets[PEB_SESSIONID];
    if ( VMI_FAILURE == vmi_read_addr(vmi, &ctx, &userid) )
        return -1;

    /* It should be safe to stash userid into a int64_t as it seldom goes above INT_MAX */
    if ( userid > INT_MAX )
        PRINT_DEBUG("The process at 0x%" PRIx64 " has a userid larger then INT_MAX!\n", eprocess_base);

    return (int64_t)userid;
};

unicode_string_t* win_get_process_csdversion(drakvuf_t drakvuf, addr_t eprocess_base)
{
    addr_t peb;
    vmi_instance_t vmi = drakvuf->vmi;
    ACCESS_CONTEXT(ctx,
        .translate_mechanism = VMI_TM_PROCESS_DTB
    );

    if (!eprocess_base)
        return NULL;

    if (VMI_FAILURE == vmi_read_addr_va(vmi, eprocess_base + drakvuf->offsets[EPROCESS_PEB], 0, &peb))
        return NULL;

    if (VMI_FAILURE == vmi_read_addr_va(vmi, eprocess_base + drakvuf->offsets[EPROCESS_PDBASE], 0, &ctx.dtb))
        return NULL;

    addr_t csdversion_va = 0;
    ctx.addr = peb + drakvuf->offsets[PEB_CSDVERSION];
    if ( VMI_FAILURE == vmi_read_addr(vmi, &ctx, &csdversion_va) )
        return NULL;

    return drakvuf_read_unicode_va(drakvuf, csdversion_va, 0);
};

int64_t win_get_current_process_userid(drakvuf_t drakvuf, drakvuf_trap_info_t* info)
{
    return win_get_process_userid(drakvuf, win_get_current_process(drakvuf, info));
}

bool win_get_process_dtb(drakvuf_t drakvuf, addr_t process_base, addr_t* dtb)
{
    if (VMI_FAILURE == vmi_read_addr_va(drakvuf->vmi, process_base + drakvuf->offsets[EPROCESS_PDBASE], 0, dtb))
        return false;
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////

static bool win_get_thread_id(drakvuf_t drakvuf, addr_t ethread, uint32_t* thread_id)
{
    addr_t p_tid ;
    if ( vmi_read_addr_va( drakvuf->vmi, ethread + drakvuf->offsets[ ETHREAD_CID ] + drakvuf->offsets[ CLIENT_ID_UNIQUETHREAD ],
            0,
            &p_tid ) == VMI_SUCCESS )
    {
        *thread_id = p_tid;

        return true;
    }

    return false ;
}


bool win_get_current_thread_id(drakvuf_t drakvuf, drakvuf_trap_info_t* info, uint32_t* thread_id)
{
    addr_t ethread = win_get_current_thread(drakvuf, info);

    if ( ethread )
        return win_get_thread_id(drakvuf, ethread, thread_id);

    return false ;
}

/////////////////////////////////////////////////////////////////////////////////////////////

// Microsoft PreviousMode KTHREAD explanation:
// https://msdn.microsoft.com/en-us/library/windows/hardware/ff559860(v=vs.85).aspx

bool win_get_thread_previous_mode( drakvuf_t drakvuf, addr_t kthread, privilege_mode_t* previous_mode )
{
    if ( kthread )
    {
        if ( vmi_read_8_va( drakvuf->vmi, kthread + drakvuf->offsets[ KTHREAD_PREVIOUSMODE ], 0,
                (uint8_t*)previous_mode ) == VMI_SUCCESS )
        {
            if ( ( *previous_mode == KERNEL_MODE ) || ( *previous_mode == USER_MODE ) )
                return true ;
        }
    }

    return false ;
}

bool win_get_current_thread_previous_mode(drakvuf_t drakvuf,
    drakvuf_trap_info_t* info,
    privilege_mode_t* previous_mode )
{
    addr_t kthread = win_get_current_thread(drakvuf, info);

    return win_get_thread_previous_mode(drakvuf, kthread, previous_mode);
}


/////////////////////////////////////////////////////////////////////////////////////////////


bool win_is_ethread( drakvuf_t drakvuf, addr_t dtb, addr_t ethread_addr )
{
    dispatcher_object_t dispatcher_type = __DISPATCHER_INVALID_OBJECT;
    ACCESS_CONTEXT(ctx,
        .translate_mechanism = VMI_TM_PROCESS_DTB,
        .dtb = dtb
    );

    ctx.addr = ethread_addr + drakvuf->offsets[ ETHREAD_TCB ] + drakvuf->offsets[ KTHREAD_HEADER ]
        + drakvuf->offsets[ DISPATCHER_TYPE ] ;

    if ( vmi_read_8( drakvuf->vmi, &ctx, (uint8_t*)&dispatcher_type ) == VMI_SUCCESS )
    {
        if ( dispatcher_type == DISPATCHER_THREAD_OBJECT )
            return true ;
    }

    return false ;
}


/////////////////////////////////////////////////////////////////////////////////////////////


bool win_is_eprocess( drakvuf_t drakvuf, addr_t dtb, addr_t eprocess_addr )
{
    dispatcher_object_t dispatcher_type = __DISPATCHER_INVALID_OBJECT;
    ACCESS_CONTEXT(ctx,
        .translate_mechanism = VMI_TM_PROCESS_DTB,
        .dtb = dtb,
        .addr = eprocess_addr + drakvuf->offsets[ EPROCESS_PCB ] + drakvuf->offsets[ KPROCESS_HEADER ]
            + drakvuf->offsets[ DISPATCHER_TYPE ]
    );

    if ( vmi_read_8( drakvuf->vmi, &ctx, (uint8_t*)&dispatcher_type ) == VMI_SUCCESS )
    {
        if ( dispatcher_type == DISPATCHER_PROCESS_OBJECT )
            return true ;
    }

    return false ;
}

bool win_is_process_suspended(drakvuf_t drakvuf, addr_t process, bool* status)
{
    vmi_instance_t vmi = drakvuf->vmi;

    addr_t list_head;
    if ( VMI_SUCCESS != vmi_read_addr_va(vmi, process + drakvuf->offsets[EPROCESS_LISTTHREADHEAD], 0, &list_head) )
        return false;

    addr_t current_list_entry = list_head;
    addr_t next_list_entry;
    if ( VMI_SUCCESS != vmi_read_addr_va(vmi, current_list_entry, 0, &next_list_entry) )
    {
        PRINT_DEBUG("Failed to read next pointer in loop at %"PRIx64"\n", current_list_entry);
        return false;
    }

    do
    {
        addr_t current_thread = current_list_entry - drakvuf->offsets[ETHREAD_THREADLISTENTRY] ;
        uint8_t thread_state;
        if ( VMI_SUCCESS != vmi_read_8_va(vmi, current_thread + drakvuf->offsets[KTHREAD_STATE], 0, &thread_state) )
        {
            PRINT_DEBUG("Failed to read thread state of thread at %"PRIx64"\n", current_thread);
            return false;
        }
        /*
         * typedef enum _KTHREAD_STATE {
         *     Initialized   = 0,
         *     Ready         = 1,
         *     Running       = 2,
         *     Standby       = 3,
         *     Terminated    = 4,
         *     Waiting       = 5,
         *     Transition    = 6,
         *     DeferredReady = 7,
         *     GateWait      = 8
         * } KTHREAD_STATE;
         */
        if (thread_state == 2) // TODO Add value
        {
            *status = false;
            return true;
        }

        current_list_entry = next_list_entry;

        if ( VMI_SUCCESS != vmi_read_addr_va(vmi, current_list_entry, 0, &next_list_entry) )
        {
            PRINT_DEBUG("Failed to read next pointer in loop at %"PRIx64"\n", current_list_entry);
            return false;
        }
    } while (next_list_entry != list_head);

    *status = true;
    return true;
}

bool win_get_module_list(drakvuf_t drakvuf, addr_t eprocess_base, addr_t* module_list)
{
    vmi_instance_t vmi = drakvuf->vmi;
    addr_t peb=0;
    addr_t ldr=0;
    addr_t modlist=0;

    ACCESS_CONTEXT(ctx,
        .translate_mechanism = VMI_TM_PROCESS_DTB
    );

    if (!eprocess_base)
        return false;

    if (VMI_FAILURE == vmi_read_addr_va(vmi, eprocess_base + drakvuf->offsets[EPROCESS_PDBASE], 0, &ctx.dtb))
        return false;

    if (VMI_FAILURE == vmi_read_addr_va(vmi, eprocess_base + drakvuf->offsets[EPROCESS_PEB], 0, &peb))
        return false;

    ctx.addr = peb + drakvuf->offsets[PEB_LDR];
    if (VMI_FAILURE == vmi_read_addr(vmi, &ctx, &ldr))
        return false;

    ctx.addr = ldr + drakvuf->offsets[PEB_LDR_DATA_INLOADORDERMODULELIST];
    if (VMI_FAILURE == vmi_read_addr(vmi, &ctx, &modlist))
        return false;

    if (!modlist)
        return false;

    *module_list = modlist;

    return true;
}

bool win_get_module_list_wow(drakvuf_t drakvuf, access_context_t* ctx, addr_t wow_peb, addr_t* module_list)
{
    if (!wow_peb || !drakvuf->wow_offsets)
        return false;

    vmi_instance_t vmi = drakvuf->vmi;

    ctx->addr = wow_peb + drakvuf->wow_offsets[WOW_PEB_LDR];

    uint32_t ldr = 0;
    if (VMI_FAILURE == vmi_read_32(vmi, ctx, &ldr))
        return false;

    ctx->addr = (addr_t)ldr + drakvuf->wow_offsets[WOW_PEB_LDR_DATA_INLOADORDERMODULELIST];

    uint32_t modlist = 0;
    if (VMI_FAILURE == vmi_read_32(vmi, ctx, &modlist))
        return false;

    if (!modlist)
        return false;

    if (module_list)
        *module_list = modlist;

    return true;
}


static bool win_find_process_list(drakvuf_t drakvuf, addr_t* list_head)
{
    vmi_instance_t vmi = drakvuf->vmi;

    addr_t current_process;
    status_t status = vmi_read_addr_ksym(vmi, "PsInitialSystemProcess", &current_process);
    if ( VMI_FAILURE == status )
        return false;

    *list_head = current_process + drakvuf->offsets[EPROCESS_TASKS];
    return true;
}

static bool win_find_driver_list(drakvuf_t drakvuf, addr_t* list_head)
{
    vmi_instance_t vmi = drakvuf->vmi;

    status_t status = vmi_read_addr_ksym(vmi, "PsLoadedModuleList", list_head);
    if ( VMI_FAILURE == status )
        return false;

    return true;
}

static bool win_find_next_process_list_entry(drakvuf_t drakvuf, addr_t current_list_entry, addr_t* next_list_entry)
{
    if ( VMI_SUCCESS == vmi_read_addr_va(drakvuf->vmi, current_list_entry, 0, next_list_entry) )
        return true;

    return false;
}

static addr_t win_process_list_entry_to_process(drakvuf_t drakvuf, addr_t list_entry)
{
    return list_entry - drakvuf->offsets[EPROCESS_TASKS];
}

bool win_find_eprocess(drakvuf_t drakvuf, vmi_pid_t find_pid, const char* find_procname, addr_t* eprocess_addr)
{
    addr_t list_head;
    if (!win_find_process_list(drakvuf, &list_head))
        return false;
    addr_t current_list_entry = list_head;
    addr_t next_list_entry;
    if (!win_find_next_process_list_entry(drakvuf, current_list_entry, &next_list_entry))
    {
        PRINT_DEBUG("Failed to read next pointer at 0x%"PRIx64" before entering loop\n", current_list_entry);
        return false;
    }

    do
    {
        vmi_pid_t pid;
        addr_t current_process = current_list_entry - drakvuf->offsets[EPROCESS_TASKS] ;

        if (!win_get_process_pid(drakvuf, current_process, &pid))
        {
            PRINT_DEBUG("Failed to read PID of process at %"PRIx64"\n", current_process);
            return false;
        }

        char* procname = win_get_process_name(drakvuf, current_process, false);

        if ((find_pid != ~0 && pid == find_pid) || (find_procname && procname && !strcasecmp(procname, find_procname)))
        {
            *eprocess_addr = current_process;
            if ( procname )
                free(procname);
            return true;
        }

        free(procname);

        current_list_entry = next_list_entry;

        if (!win_find_next_process_list_entry(drakvuf, current_list_entry, &next_list_entry))
        {
            PRINT_DEBUG("Failed to read next pointer in loop at %"PRIx64"\n", current_list_entry);
            return false;
        }

    } while (next_list_entry != list_head);

    return false;
}

bool win_search_modules( drakvuf_t drakvuf,
    const char* module_name,
    bool (*visitor_func)(drakvuf_t drakvuf, const module_info_t* module_info, void* visitor_ctx),
    void* visitor_ctx,
    addr_t eprocess_addr,
    addr_t wow_process,
    vmi_pid_t pid,
    access_context_t* ctx )
{
    bool ret = false ;
    addr_t module_list_head;

    // List x64 modules...
    if ( win_get_module_list( drakvuf, eprocess_addr, &module_list_head ) )
    {
        module_info_t* module_info = win_get_module_info_ctx( drakvuf, module_list_head, ctx, module_name );

        if ( module_info )
        {
            module_info->eprocess_addr  = eprocess_addr ;
            module_info->dtb            = ctx->dtb ;
            module_info->pid            = pid ;
            module_info->is_wow_process = wow_process ? true : false ;
            module_info->is_wow         = false ;

            ret = visitor_func( drakvuf, module_info, visitor_ctx );

            vmi_free_unicode_str( module_info->full_name );
            vmi_free_unicode_str( module_info->base_name );
            g_slice_free(module_info_t, module_info);
        }
    }

    return ret ;
}

bool win_search_modules_wow( drakvuf_t drakvuf,
    const char* module_name,
    bool (*visitor_func)(drakvuf_t drakvuf, const module_info_t* module_info, void* visitor_ctx),
    void* visitor_ctx,
    addr_t eprocess_addr,
    addr_t wow_peb,
    vmi_pid_t pid,
    access_context_t* ctx )
{
    bool ret = false ;
    addr_t module_list_head ;

    if ( win_get_module_list_wow( drakvuf, ctx, wow_peb, &module_list_head ) )
    {
        module_info_t* module_info = win_get_module_info_ctx_wow( drakvuf, module_list_head, ctx, module_name );

        if ( module_info )
        {
            module_info->eprocess_addr  = eprocess_addr ;
            module_info->dtb            = ctx->dtb ;
            module_info->pid            = pid ;
            module_info->is_wow_process = true ;
            module_info->is_wow         = true ;

            ret = visitor_func( drakvuf, module_info, visitor_ctx );

            vmi_free_unicode_str( module_info->full_name );
            vmi_free_unicode_str( module_info->base_name );
            g_slice_free(module_info_t, module_info);
        }
    }

    return ret ;
}

struct enumerate_modules_visitor_ctx
{
    addr_t eprocess;
    addr_t dtb;
    vmi_pid_t pid;
    bool is_wow_process;
    bool is_wow;
    process_const_module_visitor_t* inner_func;
    void* inner_ctx;
};

static bool enumerate_modules_visitor(drakvuf_t drakvuf, module_info_t* module_info, bool* need_free, bool* need_stop, void* visitor_ctx)
{
    struct enumerate_modules_visitor_ctx* ctx = (struct enumerate_modules_visitor_ctx*)visitor_ctx;

    module_info->eprocess_addr  = ctx->eprocess;
    module_info->dtb            = ctx->dtb;
    module_info->pid            = ctx->pid;
    module_info->is_wow_process = ctx->is_wow_process;
    module_info->is_wow         = ctx->is_wow;

    return ctx->inner_func(drakvuf, module_info, need_free, need_stop, ctx->inner_ctx);
}

bool win_enumerate_process_modules(drakvuf_t drakvuf, addr_t eprocess, process_const_module_visitor_t visitor_func, void* visitor_ctx)
{
    vmi_instance_t vmi = drakvuf->vmi;

    vmi_pid_t pid;
    addr_t dtb;
    if (win_get_process_pid(drakvuf, eprocess, &pid) &&
        vmi_pid_to_dtb(vmi, pid, &dtb) == VMI_SUCCESS)
    {
        addr_t module_list_head;
        ACCESS_CONTEXT(ctx,
            .translate_mechanism = VMI_TM_PROCESS_DTB,
            .dtb = dtb
        );
        addr_t wow_peb = win_get_wow_peb(drakvuf, &ctx, eprocess);

        // List x64 modules...
        if (win_get_module_list(drakvuf, eprocess, &module_list_head))
        {
            struct enumerate_modules_visitor_ctx enumeration_ctx =
            {
                .eprocess       = eprocess,
                .dtb            = dtb,
                .pid            = pid,
                .is_wow_process = wow_peb,
                .is_wow         = false,
                .inner_func     = visitor_func,
                .inner_ctx      = visitor_ctx,
            };

            if (!win_enumerate_module_info_ctx(drakvuf, module_list_head, &ctx, enumerate_modules_visitor, &enumeration_ctx))
                return false;
        }

        // List WoW64 modules...
        if (wow_peb && win_get_module_list_wow(drakvuf, &ctx, wow_peb, &module_list_head))
        {
            struct enumerate_modules_visitor_ctx enumeration_ctx =
            {
                .eprocess       = eprocess,
                .dtb            = dtb,
                .pid            = pid,
                .is_wow_process = true,
                .is_wow         = true,
                .inner_func     = visitor_func,
                .inner_ctx      = visitor_ctx,
            };

            if (!win_enumerate_module_info_ctx_wow(drakvuf, module_list_head, &ctx, enumerate_modules_visitor, &enumeration_ctx))
                return false;
        }
    }

    return true;
}

addr_t win_get_wow_peb( drakvuf_t drakvuf, access_context_t* ctx, addr_t eprocess )
{
    // 'Wow64Process' could not be the first member of '_EPROCESS' so this is cheap check
    if (!drakvuf->offsets[EPROCESS_WOW64PROCESS] && !drakvuf->offsets[EPROCESS_WOW64PROCESS_WIN10])
        return 0;

    addr_t ret_peb_addr = 0 ;
    addr_t wow_process = 0 ;
    addr_t eprocess_wow64_addr = eprocess + drakvuf->offsets[EPROCESS_WOW64PROCESS];

    if ( vmi_get_winver( drakvuf->vmi ) == VMI_OS_WINDOWS_10 )
        eprocess_wow64_addr = eprocess + drakvuf->offsets[EPROCESS_WOW64PROCESS_WIN10];

    if ( vmi_read_addr_va( drakvuf->vmi, eprocess_wow64_addr, 0, &wow_process ) == VMI_SUCCESS )
    {
        if ( vmi_get_winver( drakvuf->vmi ) == VMI_OS_WINDOWS_10 )
        {
            ctx->addr = wow_process + drakvuf->offsets[EWOW64PROCESS_PEB] ;

            if ( vmi_read_addr( drakvuf->vmi, ctx, &ret_peb_addr ) == VMI_FAILURE )
                ret_peb_addr = 0;
        }
        else
            ret_peb_addr = wow_process ;
    }

    return ret_peb_addr ;
}

// see https://github.com/mic101/windows/blob/master/WRK-v1.2/public/internal/base/inc/wow64tls.h#L23
#define WOW64_TLS_CPURESERVED 1

// magic offset in undocumented structure
#define WOW64_CONTEXT_PAD 4

bool win_get_wow_context(drakvuf_t drakvuf, addr_t ethread, addr_t* wow_ctx)
{
    addr_t teb_ptr;

    ACCESS_CONTEXT(ctx,
        .translate_mechanism = VMI_TM_PROCESS_PID,
        .addr = ethread + drakvuf->offsets[KTHREAD_TEB]
    );

    if (vmi_read_addr(drakvuf->vmi, &ctx, &teb_ptr) != VMI_SUCCESS)
        return false;

    addr_t eprocess;
    ctx.addr = ethread + drakvuf->offsets[KTHREAD_PROCESS];

    if (vmi_read_addr(drakvuf->vmi, &ctx, &eprocess) != VMI_SUCCESS)
        return false;

    addr_t wow64process;
    ctx.addr = eprocess + drakvuf->offsets[EPROCESS_WOW64PROCESS];

    if ( vmi_get_winver( drakvuf->vmi ) == VMI_OS_WINDOWS_10 )
        ctx.addr = eprocess + drakvuf->offsets[EPROCESS_WOW64PROCESS_WIN10];

    if (vmi_read_addr(drakvuf->vmi, &ctx, &wow64process) != VMI_SUCCESS)
        return false;

    // seems like process is not a WOW64 process, so the data in TLS may be fake
    if (!wow64process)
        return false;

    pid_t pid;

    if (!win_get_process_pid(drakvuf, eprocess, &pid))
        return false;

    ctx.translate_mechanism = VMI_TM_PROCESS_DTB;

    if (vmi_pid_to_dtb(drakvuf->vmi, pid, &ctx.dtb) != VMI_SUCCESS)
        return false;

    addr_t self_teb_ptr;
    if (vmi_read_addr(drakvuf->vmi, &ctx, &self_teb_ptr) != VMI_SUCCESS)
        return false;

    addr_t tls_slot;
    // like: NtCurrentTeb()->TlsSlots[WOW64_TLS_CPURESERVED]
    tls_slot = teb_ptr + drakvuf->offsets[TEB_TLS_SLOTS] + (WOW64_TLS_CPURESERVED * sizeof(uint64_t));

    addr_t tls_slot_val;
    ctx.addr = tls_slot;

    if (vmi_read_addr(drakvuf->vmi, &ctx, &tls_slot_val) != VMI_SUCCESS)
        return false;

    *wow_ctx = tls_slot_val + WOW64_CONTEXT_PAD;
    return true;
}

bool win_get_user_stack64(drakvuf_t drakvuf, drakvuf_trap_info_t* info, addr_t* stack_ptr)
{
    addr_t ptrap_frame;
    uint64_t rsp;

    ACCESS_CONTEXT(ctx,
        .translate_mechanism = VMI_TM_PROCESS_PID,
        .addr = win_get_current_thread(drakvuf, info) + drakvuf->offsets[KTHREAD_TRAPFRAME]
    );

    if (vmi_read_addr(drakvuf->vmi, &ctx, &ptrap_frame) != VMI_SUCCESS)
        return false;

    ctx.addr = ptrap_frame + drakvuf->offsets[KTRAP_FRAME_RSP];

    if (vmi_read_64(drakvuf->vmi, &ctx, &rsp) != VMI_SUCCESS)
        return false;

    *stack_ptr = rsp;
    return true;
}

bool win_get_user_stack32(drakvuf_t drakvuf, drakvuf_trap_info_t* info, addr_t* stack_ptr, addr_t* frame_ptr)
{
    uint32_t esp;
    uint32_t ebp;

    addr_t wow_ctx;

    if (!drakvuf->wow_offsets)
        return false;

    if (!win_get_wow_context(drakvuf, win_get_current_thread(drakvuf, info), &wow_ctx))
        return false;

    ACCESS_CONTEXT(ctx,
        .translate_mechanism = VMI_TM_PROCESS_DTB,
        .dtb = info->regs->cr3,
        .addr = wow_ctx + drakvuf->wow_offsets[WOW_CONTEXT_ESP]
    );

    if (vmi_read_32(drakvuf->vmi, &ctx, &esp) != VMI_SUCCESS)
        return false;

    ctx.addr = wow_ctx + drakvuf->wow_offsets[WOW_CONTEXT_EBP];

    if (vmi_read_32(drakvuf->vmi, &ctx, &ebp) != VMI_SUCCESS)
        return false;

    *stack_ptr = esp;
    *frame_ptr = ebp;
    return true;
}

bool win_enumerate_processes( drakvuf_t drakvuf, void (*visitor_func)(drakvuf_t drakvuf, addr_t eprocess, void* visitor_ctx), void* visitor_ctx )
{
    addr_t list_head;
    if (!win_find_process_list(drakvuf, &list_head))
        return false;
    addr_t current_list_entry = list_head;
    addr_t next_list_entry;

    if (!win_find_next_process_list_entry(drakvuf, current_list_entry, &next_list_entry))
    {
        PRINT_DEBUG("Failed to read next pointer at 0x%"PRIx64" before entering loop\n", current_list_entry);
        return false;
    }

    do
    {
        addr_t eprocess = win_process_list_entry_to_process(drakvuf, current_list_entry);

        visitor_func(drakvuf, eprocess, visitor_ctx);

        current_list_entry = next_list_entry;

        if (!win_find_next_process_list_entry(drakvuf, current_list_entry, &next_list_entry))
        {
            PRINT_DEBUG("Failed to read next pointer in loop at %"PRIx64"\n", current_list_entry);
            return false;
        }
    } while (next_list_entry != list_head);

    return true;
}

bool win_enumerate_drivers( drakvuf_t drakvuf, void (*visitor_func)(drakvuf_t drakvuf, addr_t eprocess, void* visitor_ctx), void* visitor_ctx )
{
    addr_t list_head;
    if (!win_find_driver_list(drakvuf, &list_head))
        return false;
    addr_t current_list_entry = list_head;
    addr_t next_list_entry;

    if (!win_find_next_process_list_entry(drakvuf, current_list_entry, &next_list_entry))
    {
        PRINT_DEBUG("Failed to read next pointer at 0x%"PRIx64" before entering loop\n", current_list_entry);
        return false;
    }

    do
    {
        visitor_func(drakvuf, current_list_entry, visitor_ctx);

        current_list_entry = next_list_entry;

        if (!win_find_next_process_list_entry(drakvuf, current_list_entry, &next_list_entry))
        {
            PRINT_DEBUG("Failed to read next pointer in loop at %"PRIx64"\n", current_list_entry);
            return false;
        }
    } while (next_list_entry != list_head);

    return true;
}

bool win_enumerate_processes_with_module( drakvuf_t drakvuf, const char* module_name, bool (*visitor_func)(drakvuf_t drakvuf, const module_info_t* module_info, void* visitor_ctx), void* visitor_ctx )
{
    addr_t list_head;
    if (!win_find_process_list(drakvuf, &list_head))
        return false;
    addr_t current_list_entry = list_head;
    addr_t next_list_entry;

    if (!win_find_next_process_list_entry(drakvuf, current_list_entry, &next_list_entry))
    {
        PRINT_DEBUG("Failed to read next pointer at 0x%"PRIx64" before entering loop\n", current_list_entry);
        return false;
    }

    do
    {
        addr_t current_process = win_process_list_entry_to_process(drakvuf, current_list_entry);

        vmi_pid_t pid ;

        if ( win_get_process_pid( drakvuf, current_process, &pid) )
        {
            ACCESS_CONTEXT(ctx,
                .translate_mechanism = VMI_TM_PROCESS_DTB
            );

            if ( vmi_pid_to_dtb( drakvuf->vmi, pid, &ctx.dtb ) == VMI_SUCCESS )
            {
                addr_t wow_peb = win_get_wow_peb( drakvuf, &ctx, current_process ) ;

                if ( win_search_modules( drakvuf, module_name, visitor_func, visitor_ctx, current_process,
                        wow_peb, pid, &ctx ) )
                    return true ;

                // List WoW64 modules...
                if ( wow_peb )
                {
                    if ( win_search_modules_wow( drakvuf, module_name, visitor_func, visitor_ctx, current_process,
                            wow_peb, pid, &ctx ) )
                        return true ;
                }
            }
        }

        current_list_entry = next_list_entry;

        if (!win_find_next_process_list_entry(drakvuf, current_list_entry, &next_list_entry))
        {
            PRINT_DEBUG("Failed to read next pointer in loop at %"PRIx64"\n", current_list_entry);
            return false;
        }
    } while (next_list_entry != list_head);

    return false;
}

bool win_is_crashreporter(drakvuf_t drakvuf, drakvuf_trap_info_t* info, vmi_pid_t* pid)
{
    if (sizeof("WerFault.exe") - 1 > strlen(info->proc_data.name))
        return false;

    if (!strstr(info->proc_data.name, "WerFault.exe"))
        return false;

    char* cmdline = win_get_process_commandline(drakvuf, info, info->proc_data.base_addr);
    if (!cmdline)
    {
        PRINT_DEBUG("Error. Failed to get command line\n");
        return false;
    }

    char* param = strstr(cmdline, "-p ");
    if (!param)
    {
        PRINT_DEBUG("Error. Failed to get param\n");
        free(cmdline);
        return false;
    }

    char* end = NULL;
    *pid = strtoul(param + 3, &end, 10);
    if (ERANGE == errno)
    {
        PRINT_DEBUG("Error. Failed to parse PID: the value is out of range\n");
        free(cmdline);
        return false;
    }

    free(cmdline);
    return true;
}

////////////////////////////////////////////////////////////////

bool win_get_process_ppid( drakvuf_t drakvuf, addr_t process_base, vmi_pid_t* ppid )
{
    if ( VMI_SUCCESS == vmi_read_32_va( drakvuf->vmi, process_base + drakvuf->offsets[EPROCESS_INHERITEDPID], 0, (uint32_t*)ppid ) )
        return true;

    return false;
}

bool win_get_process_data( drakvuf_t drakvuf, addr_t base_addr, proc_data_priv_t* proc_data )
{
    proc_data->base_addr = base_addr;

    if ( base_addr )
    {
        if ( win_get_process_pid( drakvuf, base_addr, &proc_data->pid ) )
        {
            if ( win_get_process_ppid( drakvuf, base_addr, &proc_data->ppid ) )
            {
                if (drakvuf->get_userid)
                    proc_data->userid = win_get_process_userid( drakvuf, base_addr );
                else
                    proc_data->userid = 0;
                proc_data->name   = win_get_process_name( drakvuf, base_addr, true );

                if ( proc_data->name )
                    return true;
            }
        }
    }

    return false;
}

static bool win_parse_mmvad(drakvuf_t drakvuf, addr_t node_addr, addr_t* left_child, addr_t* right_child, uint64_t* starting_vpn, uint64_t* ending_vpn, uint64_t* mmvad_core)
{
    bool is_win7 = vmi_get_winver( drakvuf->vmi ) <= VMI_OS_WINDOWS_7;

    ACCESS_CONTEXT(ctx,
        .translate_mechanism = VMI_TM_PROCESS_PID,
        .pid = 4
    );

    uint32_t starting_vpn_low;
    uint32_t ending_vpn_low;
    uint8_t starting_vpn_high;
    uint8_t ending_vpn_high;

    if (is_win7)
    {
        ctx.addr = node_addr + drakvuf->offsets[MMVAD_LEFT_CHILD];
    }
    else
    {
        ctx.addr = node_addr + drakvuf->offsets[RTL_BALANCED_NODE_LEFT];
    }

    if (vmi_read_addr(drakvuf->vmi, &ctx, left_child) != VMI_SUCCESS)
    {
        return false;
    }

    if (is_win7)
    {
        ctx.addr = node_addr + drakvuf->offsets[MMVAD_RIGHT_CHILD];
    }
    else
    {
        ctx.addr = node_addr + drakvuf->offsets[RTL_BALANCED_NODE_RIGHT];
    }

    if (vmi_read_addr(drakvuf->vmi, &ctx, right_child) != VMI_SUCCESS)
    {
        return false;
    }

    if (is_win7)
    {
        ctx.addr = node_addr + drakvuf->offsets[MMVAD_STARTING_VPN];
        if (vmi_read_addr(drakvuf->vmi, &ctx, starting_vpn) != VMI_SUCCESS)
            return false;

        ctx.addr = node_addr + drakvuf->offsets[MMVAD_ENDING_VPN];
        if (vmi_read_addr(drakvuf->vmi, &ctx, ending_vpn) != VMI_SUCCESS)
            return false;
    }
    else
    {
        *mmvad_core = node_addr + drakvuf->offsets[MMVAD_CORE];
        ctx.addr = *mmvad_core + drakvuf->offsets[MMVAD_SHORT_STARTING_VPN];

        if (vmi_read_32(drakvuf->vmi, &ctx, &starting_vpn_low) != VMI_SUCCESS)
        {
            return false;
        }

        ctx.addr = *mmvad_core + drakvuf->offsets[MMVAD_SHORT_ENDING_VPN];

        if (vmi_read_32(drakvuf->vmi, &ctx, &ending_vpn_low) != VMI_SUCCESS)
        {
            return false;
        }

        ctx.addr = *mmvad_core + drakvuf->offsets[MMVAD_SHORT_STARTING_VPN_HIGH];

        if (vmi_read_8(drakvuf->vmi, &ctx, &starting_vpn_high) != VMI_SUCCESS)
        {
            return false;
        }

        ctx.addr = *mmvad_core + drakvuf->offsets[MMVAD_SHORT_ENDING_VPN_HIGH];

        if (vmi_read_8(drakvuf->vmi, &ctx, &ending_vpn_high) != VMI_SUCCESS)
        {
            return false;
        }

        *starting_vpn = starting_vpn_high;
        *starting_vpn <<= 32;
        *starting_vpn |= starting_vpn_low;

        *ending_vpn = ending_vpn_high;
        *ending_vpn <<= 32;
        *ending_vpn |= ending_vpn_low;
    }

    return true;
}

bool win_find_mmvad(drakvuf_t drakvuf, addr_t eprocess, addr_t vaddr, mmvad_info_t* out_mmvad)
{
    int depth = 0;
    bool is_win7 = vmi_get_winver( drakvuf->vmi ) <= VMI_OS_WINDOWS_7;
    bool is32bit = (drakvuf_get_page_mode(drakvuf) != VMI_PM_IA32E);

    int pool_tag_delta = is32bit ? POOL_TAG_SIZE_X86 : POOL_TAG_SIZE_X64;

    ACCESS_CONTEXT(ctx,
        .translate_mechanism = VMI_TM_PROCESS_PID,
        .pid = 4
    );

    addr_t node_addr;

    if (is_win7)
    {
        node_addr = eprocess + drakvuf->offsets[EPROCESS_VADROOT] + drakvuf->offsets[VADROOT_BALANCED_ROOT];
    }
    else
    {
        ctx.addr = eprocess + drakvuf->offsets[EPROCESS_VADROOT] + drakvuf->offsets[RTL_AVL_TREE_ROOT];

        if (vmi_read_addr(drakvuf->vmi, &ctx, &node_addr) != VMI_SUCCESS)
        {
            PRINT_DEBUG("MMVAD failed for node addr\n");
            return false;
        }
    }

    while (node_addr)
    {
        if (depth > MMVAD_MAX_DEPTH)
        {
            PRINT_DEBUG("Error. Max depth exceeded when walking MMVAD tree.\n");
            return false;
        }

        ++depth;

        addr_t left_child;
        addr_t right_child;

        uint64_t starting_vpn = 0;
        uint64_t ending_vpn = 0;

        uint64_t mmvad_core = 0;

        if (!win_parse_mmvad(drakvuf, node_addr, &left_child, &right_child, &starting_vpn, &ending_vpn, &mmvad_core))
            return false;

        if (starting_vpn == 0 && ending_vpn == 0)
        {
            // the root node seems to be empty with only right child pointer filled in
            node_addr = right_child;
        }
        else if (starting_vpn * VMI_PS_4KB <= vaddr && (ending_vpn + 1) * VMI_PS_4KB > vaddr)
        {
            uint32_t pool_tag;
            addr_t subsection;
            addr_t control_area;
            addr_t segment;
            uint32_t ptes;
            addr_t prototype_pte;
            uint64_t flags = 0;
            uint32_t flags1 = 0;

            out_mmvad->file_name_ptr = 0;

            if (is_win7)
            {
                ctx.addr = node_addr + drakvuf->offsets[MMVAD_FLAGS];
                if (vmi_read_64(drakvuf->vmi, &ctx, &flags) != VMI_SUCCESS)
                {
                    return false;
                }
            }
            else
            {
                ctx.addr = node_addr + drakvuf->offsets[MMVAD_SHORT_FLAGS];
                uint32_t flags32 = 0;
                if (vmi_read_32(drakvuf->vmi, &ctx, &flags32) != VMI_SUCCESS)
                {
                    return false;
                }
                flags = flags32;
                ctx.addr = mmvad_core + drakvuf->offsets[MMVAD_SHORT_FLAGS1];
                if (vmi_read_32(drakvuf->vmi, &ctx, &flags1) != VMI_SUCCESS)
                {
                    return false;
                }
            }

            // read Windows' PoolTag which is 12 bytes before the actual object
            ctx.addr = node_addr - pool_tag_delta;
            if (vmi_read_32(drakvuf->vmi, &ctx, &pool_tag) != VMI_SUCCESS)
            {
                return false;
            }

            // Windows MMVAD can have multiple types, can be differentiated with pool tags
            // some types are shorter and don't even contain "Subsection" field
            if (pool_tag == POOL_TAG_VADL || pool_tag == POOL_TAG_VAD || pool_tag == POOL_TAG_VADM)
            {
                ctx.addr = node_addr + drakvuf->offsets[MMVAD_SUBSECTION];

                if (vmi_read_addr(drakvuf->vmi, &ctx, &subsection) == VMI_SUCCESS)
                {
                    ctx.addr = subsection + drakvuf->offsets[SUBSECTION_CONTROL_AREA];

                    if (vmi_read_addr(drakvuf->vmi, &ctx, &control_area) == VMI_SUCCESS)
                    {
                        out_mmvad->file_name_ptr = win_get_file_name_ptr_from_controlarea(drakvuf, control_area);

                        ctx.addr = control_area + drakvuf->offsets[CONTROL_AREA_SEGMENT];

                        if (vmi_read_addr(drakvuf->vmi, &ctx, &segment) == VMI_SUCCESS)
                        {
                            ctx.addr = control_area + drakvuf->offsets[SEGMENT_TOTALNUMBEROFPTES];

                            if (vmi_read_32(drakvuf->vmi, &ctx, &ptes) == VMI_SUCCESS)
                            {
                                out_mmvad->total_number_of_ptes = ptes;
                            }

                            ctx.addr = control_area + drakvuf->offsets[SEGMENT_PROTOTYPEPTE];

                            if (vmi_read_addr(drakvuf->vmi, &ctx, &prototype_pte) == VMI_SUCCESS)
                            {
                                out_mmvad->prototype_pte = prototype_pte;
                            }
                        }
                    }
                }
            }

            out_mmvad->starting_vpn = starting_vpn;
            out_mmvad->ending_vpn = ending_vpn;
            out_mmvad->flags = flags;
            out_mmvad->flags1 = flags1;

            return true;
        }
        else if (starting_vpn * VMI_PS_4KB > vaddr)
        {
            node_addr = left_child;
        }
        else
        {
            node_addr = right_child;
        }
    }

    return false;
}

static bool win_traverse_mmvad_node(drakvuf_t drakvuf, addr_t node_addr, mmvad_callback callback, void* callback_data)
{
    bool is_win7 = vmi_get_winver( drakvuf->vmi ) <= VMI_OS_WINDOWS_7;
    bool is32bit = (drakvuf_get_page_mode(drakvuf) != VMI_PM_IA32E);

    int pool_tag_delta = is32bit ? POOL_TAG_SIZE_X86 : POOL_TAG_SIZE_X64;

    ACCESS_CONTEXT(ctx,
        .translate_mechanism = VMI_TM_PROCESS_PID,
        .pid = 4
    );

    addr_t left_child;
    addr_t right_child;

    uint64_t starting_vpn = 0;
    uint64_t ending_vpn = 0;

    uint64_t mmvad_core = 0;

    if (!win_parse_mmvad(drakvuf, node_addr, &left_child, &right_child, &starting_vpn, &ending_vpn, &mmvad_core))
        return false;

    if (starting_vpn == 0 && ending_vpn == 0)
    {
        // the root node seems to be empty with only right child pointer filled in
        return win_traverse_mmvad_node(drakvuf, right_child, callback, callback_data);
    }

    uint32_t pool_tag;
    addr_t subsection;
    addr_t control_area;
    addr_t segment;
    uint32_t ptes;
    addr_t prototype_pte;
    uint64_t flags = 0;
    uint32_t flags1 = 0;

    mmvad_info_t mmvad;
    mmvad.file_name_ptr = 0;
    mmvad.total_number_of_ptes = 0;
    mmvad.prototype_pte = 0;

    if (is_win7)
    {
        ctx.addr = node_addr + drakvuf->offsets[MMVAD_FLAGS];
        if (vmi_read_64(drakvuf->vmi, &ctx, &flags) != VMI_SUCCESS)
        {
            return false;
        }
    }
    else
    {
        ctx.addr = node_addr + drakvuf->offsets[MMVAD_SHORT_FLAGS];
        uint32_t flags32 = 0;
        if (vmi_read_32(drakvuf->vmi, &ctx, &flags32) != VMI_SUCCESS)
        {
            return false;
        }
        flags = flags32;
        ctx.addr = mmvad_core + drakvuf->offsets[MMVAD_SHORT_FLAGS1];
        if (vmi_read_32(drakvuf->vmi, &ctx, &flags1) != VMI_SUCCESS)
        {
            return false;
        }
    }

    // read Windows' PoolTag which is 12 bytes before the actual object
    ctx.addr = node_addr - pool_tag_delta;
    if (vmi_read_32(drakvuf->vmi, &ctx, &pool_tag) != VMI_SUCCESS)
    {
        return false;
    }

    // Windows MMVAD can have multiple types, can be differentiated with pool tags
    // some types are shorter and don't even contain "Subsection" field
    if (pool_tag == POOL_TAG_VADL || pool_tag == POOL_TAG_VAD || pool_tag == POOL_TAG_VADM)
    {
        ctx.addr = node_addr + drakvuf->offsets[MMVAD_SUBSECTION];

        if (vmi_read_addr(drakvuf->vmi, &ctx, &subsection) == VMI_SUCCESS)
        {
            ctx.addr = subsection + drakvuf->offsets[SUBSECTION_CONTROL_AREA];

            if (vmi_read_addr(drakvuf->vmi, &ctx, &control_area) == VMI_SUCCESS)
            {
                mmvad.file_name_ptr = win_get_file_name_ptr_from_controlarea(drakvuf, control_area);

                // FIXME We could get this fields form MMVAD.FirstPrototypePte and MMVAD.LastPrototypePte
                ctx.addr = control_area + drakvuf->offsets[CONTROL_AREA_SEGMENT];

                if (vmi_read_addr(drakvuf->vmi, &ctx, &segment) == VMI_SUCCESS)
                {
                    ctx.addr = segment + drakvuf->offsets[SEGMENT_TOTALNUMBEROFPTES];

                    if (vmi_read_32(drakvuf->vmi, &ctx, &ptes) == VMI_SUCCESS)
                    {
                        mmvad.total_number_of_ptes = ptes;
                    }

                    ctx.addr = segment + drakvuf->offsets[SEGMENT_PROTOTYPEPTE];

                    if (vmi_read_addr(drakvuf->vmi, &ctx, &prototype_pte) == VMI_SUCCESS)
                    {
                        mmvad.prototype_pte = prototype_pte;
                    }
                }
            }
        }
    }

    mmvad.starting_vpn = starting_vpn;
    mmvad.ending_vpn = ending_vpn;
    mmvad.flags = flags;
    mmvad.flags1 = flags1;

    if (win_traverse_mmvad_node(drakvuf, left_child, callback, callback_data))
        return true;

    if (callback(drakvuf, &mmvad, callback_data))
        return true;

    return win_traverse_mmvad_node(drakvuf, right_child, callback, callback_data);
}

bool win_traverse_mmvad(drakvuf_t drakvuf, addr_t eprocess, mmvad_callback callback, void* callback_data)
{
    bool is_win7 = vmi_get_winver( drakvuf->vmi ) <= VMI_OS_WINDOWS_7;

    ACCESS_CONTEXT(ctx,
        .translate_mechanism = VMI_TM_PROCESS_PID,
        .pid = 4
    );

    addr_t node_addr = 0;

    if (is_win7)
    {
        node_addr = eprocess + drakvuf->offsets[EPROCESS_VADROOT] + drakvuf->offsets[VADROOT_BALANCED_ROOT];
    }
    else
    {
        ctx.addr = eprocess + drakvuf->offsets[EPROCESS_VADROOT] + drakvuf->offsets[RTL_AVL_TREE_ROOT];

        if (vmi_read_addr(drakvuf->vmi, &ctx, &node_addr) != VMI_SUCCESS)
        {
            PRINT_DEBUG("MMVAD failed for node addr\n");
            return false;
        }
    }

    if (node_addr)
        return win_traverse_mmvad_node(drakvuf, node_addr, callback, callback_data);

    return false;
}

static uint64_t win_mmvad_flag(drakvuf_t drakvuf, uint64_t flags, int idx)
{
    bitfield_t bf = &drakvuf->bitfields[idx];
    uint64_t mask = (1ULL << (bf->end_bit - bf->start_bit)) - 1;
    return (flags >> bf->start_bit) & mask;
}

bool win_is_mmvad_commited(drakvuf_t drakvuf, mmvad_info_t* mmvad)
{
    bool is_win7 = vmi_get_winver( drakvuf->vmi ) <= VMI_OS_WINDOWS_7;
    int idx = is_win7 ? MMVAD_FLAGS_MEMCOMMIT : MMVAD_FLAGS1_MEMCOMMIT;
    uint64_t flags = is_win7 ? mmvad->flags : mmvad->flags1;
    return win_mmvad_flag(drakvuf, flags, idx);
}

uint64_t win_mmvad_commit_charge(drakvuf_t drakvuf, mmvad_info_t* mmvad, uint64_t* width)
{
    bool is_win7 = vmi_get_winver( drakvuf->vmi ) <= VMI_OS_WINDOWS_7;
    int idx = is_win7 ? MMVAD_FLAGS_COMMITCHARGE : MMVAD_FLAGS1_COMMITCHARGE;
    uint64_t flags = is_win7 ? mmvad->flags : mmvad->flags1;
    if (width)
        *width = drakvuf->bitfields[idx].end_bit - drakvuf->bitfields[idx].start_bit;
    return win_mmvad_flag(drakvuf, flags, idx);
}

uint32_t win_mmvad_type(drakvuf_t drakvuf, mmvad_info_t* mmvad)
{
    int idx = MMVAD_FLAGS_VADTYPE;
    return win_mmvad_flag(drakvuf, mmvad->flags, idx);
}

bool win_get_pid_from_handle(drakvuf_t drakvuf, drakvuf_trap_info_t* info, addr_t handle, vmi_pid_t* pid)
{
    if (handle == 0 || handle == UINT64_MAX)
    {
        *pid = info->proc_data.pid;
        return false;
    }

    if (!info->proc_data.base_addr)
    {
        return false;
    }

    addr_t obj = drakvuf_get_obj_by_handle(drakvuf, info->proc_data.base_addr, handle);
    if (!obj)
    {
        return false;
    }

    addr_t eprocess_base = obj + drakvuf->offsets[OBJECT_HEADER_BODY];
    return drakvuf_get_process_pid(drakvuf, eprocess_base, pid);
}

bool win_get_tid_from_handle(drakvuf_t drakvuf, drakvuf_trap_info_t* info, addr_t handle, uint32_t* tid)
{
    if (handle == 0 || handle == UINT64_MAX)
    {
        *tid = info->proc_data.tid;
        return false;
    }

    if (!info->proc_data.base_addr)
    {
        return false;
    }

    addr_t obj = drakvuf_get_obj_by_handle(drakvuf, info->proc_data.base_addr, handle);
    if (!obj)
    {
        return false;
    }

    addr_t ethread = obj + drakvuf->offsets[OBJECT_HEADER_BODY];
    return win_get_thread_id(drakvuf, ethread, tid);
}
