#undef TRACE_SYSTEM
#define TRACE_SYSTEM myprot

#if !defined(_MYPROT_TRACE_H) || defined(TRACE_HEADER_MULTI_READ)
#define _MYPROT_TRACE_H
#include <linux/tracepoint.h>

TRACE_EVENT(myprot_port,
    TP_PROTO(unsigned short dest, unsigned short source),
    TP_ARGS(dest, source),
    TP_STRUCT__entry(
        __field(unsigned short, dest)
        __field(unsigned short, source)
    ),
    TP_fast_assign(
        __entry->dest = dest;
        __entry->source = source;
    ),

    TP_printk("dest:%d, source:%d", __entry->dest, __entry->source)
);

#endif
/*
 * 需要添加以下这一坨
 * 由于模块不能修改内核头文件，因此我们需要在自己的目录放头文件，
 * TRACE_INCLUDE_PATH 必须重新定义，而不是仅仅在内核头文件目录寻找定义。
 *
 * 为了支持这个重定义，Makefile中必须包含：
 * CFLAGS_myprot.o = -I$(src)
 *
 */

#undef TRACE_INCLUDE_PATH
#define TRACE_INCLUDE_PATH .
#define TRACE_INCLUDE_FILE myprot
#include <trace/define_trace.h>