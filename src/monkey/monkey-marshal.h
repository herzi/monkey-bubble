
#ifndef __monkey_marshal_MARSHAL_H__
#define __monkey_marshal_MARSHAL_H__

#include	<glib-object.h>

G_BEGIN_DECLS

/* VOID:POINTER,POINTER (monkey-marshal.list:1) */
extern void monkey_marshal_VOID__POINTER_POINTER (GClosure     *closure,
                                                  GValue       *return_value,
                                                  guint         n_param_values,
                                                  const GValue *param_values,
                                                  gpointer      invocation_hint,
                                                  gpointer      marshal_data);

/* VOID:OBJECT,OBJECT (monkey-marshal.list:2) */
extern void monkey_marshal_VOID__OBJECT_OBJECT (GClosure     *closure,
                                                GValue       *return_value,
                                                guint         n_param_values,
                                                const GValue *param_values,
                                                gpointer      invocation_hint,
                                                gpointer      marshal_data);

/* VOID:OBJECT,INT (monkey-marshal.list:3) */
extern void monkey_marshal_VOID__OBJECT_INT (GClosure     *closure,
                                             GValue       *return_value,
                                             guint         n_param_values,
                                             const GValue *param_values,
                                             gpointer      invocation_hint,
                                             gpointer      marshal_data);

/* VOID:POINTER,INT (monkey-marshal.list:4) */
extern void monkey_marshal_VOID__POINTER_INT (GClosure     *closure,
                                              GValue       *return_value,
                                              guint         n_param_values,
                                              const GValue *param_values,
                                              gpointer      invocation_hint,
                                              gpointer      marshal_data);

/* VOID:POINTER,POINTER (monkey-marshal.list:5) */

G_END_DECLS

#endif /* __monkey_marshal_MARSHAL_H__ */

