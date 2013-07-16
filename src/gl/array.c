#include "array.h"

GLvoid *copy_gl_array(const GLvoid *src,
                      GLenum from, GLsizei width, GLsizei stride,
                      GLenum to, GLsizei to_width, GLsizei count) {
    if (! src || !count)
        return NULL;

    if (! stride)
        stride = width * gl_sizeof(from);

    const char *unknown_str = "libGL: copy_gl_array -> unknown type: %x\n";
    GLvoid *dst = malloc(count * to_width * gl_sizeof(to));
    GLsizei from_size = gl_sizeof(from) * width;
    GLsizei to_size = gl_sizeof(to) * to_width;

    if (to_width < width) {
        printf("Warning: copy_gl_array: %i < %i\n", to_width, width);
        return NULL;
    }

    // if stride is weird, we need to be able to arbitrarily shift src
    // so we leave it in a uintptr_t and cast after incrementing
    uintptr_t in = (uintptr_t)src;
    if (from == to && to_width >= width) {
        GL_TYPE_SWITCH(out, dst, to,
            for (int i = 0; i < count; i++) {
                memcpy(out, (GLvoid *)in, from_size);
                for (int j = width; j < to_width; j++) {
                    out[j] = 0;
                }
                out += to_width;
                in += stride;
            },
            default:
                printf(unknown_str, from);
                return NULL;
        )
    } else {
        GL_TYPE_SWITCH(out, dst, to,
            for (int i = 0; i < count; i++) {
                GL_TYPE_SWITCH(input, in, from,
                    for (int j = 0; j < width; j++) {
                        out[j] = input[j];
                    }
                    for (int j = width; j < to_width; j++) {
                        out[j] = 0;
                    }
                    out += to_width;
                    in += stride;
                ,
                    default:
                        printf(unknown_str, from);
                        return NULL;
                )
            },
            default:
                printf(unknown_str, to);
                return NULL;
        )
    }

    return dst;
}

GLvoid *copy_gl_pointer(PointerState *ptr, GLsizei width, GLsizei count) {
    return copy_gl_array(ptr->pointer, ptr->type, ptr->size, ptr->stride,
                         GL_FLOAT, width, count);
}

GLfloat *gl_pointer_index(PointerState *p, GLint index) {
    static GLfloat buf[4];
    GLsizei size = gl_sizeof(p->type);
    GLsizei stride = p->stride ? p->stride : size * p->size;
    uintptr_t ptr = (uintptr_t)p->pointer + (stride * index);

    GL_TYPE_SWITCH(src, ptr, p->type,
        for (int i = 0; i < p->size; i++) {
            buf[i] = src[i];
        }
        // zero anything not set by the pointer
        for (int i = p->size; i < 4; i++) {
            buf[i] = 0;
        },
        default:
            printf("libGL: unknown pointer type: 0x%x\n", p->type);
    )
    return buf;
}