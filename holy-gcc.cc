/* This plugin adds support for HolyC prints */

#include "gcc-plugin.h"

#include "debug-utils.h"

#include "tree-iterator.h"

// We must assert that this plugin is GPL compatible
int plugin_is_GPL_compatible;


static bool
is_pointer_to_char_type (tree t)
{
  return POINTER_TYPE_P(t) && (TREE_TYPE(t) == char_type_node);
}

static bool
is_string_cst_addr (tree t)
{
  return (TREE_CODE (t) == ADDR_EXPR) &&
         (TREE_CODE (TREE_OPERAND (t, 0)) == STRING_CST);
}

static bool
is_holy_c_print (tree t)
{
  /* What we're looking for is a statement that contains only an address of
     string litaral being casted to (char *). I.e. something like that:
     (char *) "some string";
     In other words, we're looking for a statement which is a pointer cast
     (NOP_EXPR), that casts to (char *) and takes a STRING_CST as an
     operand.  */
  return TREE_CODE (t) == NOP_EXPR &&
         is_pointer_to_char_type (TREE_TYPE (t)) &&
         is_string_cst_addr (TREE_OPERAND (t, 0));
}

static tree
build_print_call_expr (tree str)
{
  tree fn = builtin_decl_implicit (BUILT_IN_PRINTF);
  return build_call_expr (fn, 1, str);
}

static tree
cb_walk_tree_fn (tree *tp, int *walk_subtrees,
     void *data ATTRIBUTE_UNUSED)
{
  /* TODO: Is it possible to get all STATEMENT_LISTs out of fndecl directly,
     withuot walk_tree the entire function?  */
  if (TREE_CODE (*tp) == STATEMENT_LIST)
  {
    tree_stmt_iterator tsi = tsi_start (*tp);
    while (!tsi_end_p (tsi))
    {
      tree stmt = tsi_stmt (tsi);
      if (is_holy_c_print (stmt))
      {
        /* Replace this expression with a call to a print function and pass
           it as an argument  */
        tsi_link_before (&tsi, build_print_call_expr (stmt), TSI_SAME_STMT);
        tsi_delink (&tsi);
      }
      else
      {
        /* If we performed tsi_delink() tsi already points to the next
           statement  */
        tsi_next (&tsi);
      }
    }
  }
  /* This is a special case. If a BIND_EXPR contains only one statement,
     instead of holding a STATEMENT_LIST of one element it's replaced with that
     single statement  */
  else if (TREE_CODE (*tp) == BIND_EXPR &&
           is_holy_c_print (BIND_EXPR_BODY (*tp)))
  {
    BIND_EXPR_BODY (*tp) = build_print_call_expr (BIND_EXPR_BODY (*tp));
  }

  return NULL_TREE;
}

static void
callback (void *gcc_data, void *user_data)
{
  tree fndecl = (tree)gcc_data;

  walk_tree (&DECL_SAVED_TREE (fndecl), cb_walk_tree_fn, NULL, NULL);
}

int
plugin_init (struct plugin_name_args *plugin_info,
                 struct plugin_gcc_version *version)
{
  /* After the code is gimplified, statements which contain only a string are
     already removed, so we have to enter before that  */
  register_callback ("holy_gcc",
                     PLUGIN_PRE_GENERICIZE,
                     callback,
                     NULL);

  return 0;
}
