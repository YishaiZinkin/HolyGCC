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


static tree
cb_walk_tree_fn (tree *tp, int *walk_subtrees,
     void *data ATTRIBUTE_UNUSED)
{
  /* TODO: Is it possible to get STATEMENT_LIST out of fndecl directly,
     withuot walk_tree?  */
  if (TREE_CODE (*tp) == STATEMENT_LIST)
  {
    tree_stmt_iterator i;
    for (i = tsi_start (*tp); !tsi_end_p (i); tsi_next (&i))
    {
      tree stmt = tsi_stmt (i);
      /* What we're looking for is a statement that contains only an address of
         string litaral being casted to (char *). I.e. something like that:
         (char *) "Some string";
         In other words, we're looking for a statement which is a pointer cast
         (NOP_EXPR), that casts to a (char *) and takes a STRING_CST as an
         operand.  */
      if (TREE_CODE (stmt) == NOP_EXPR &&
          is_pointer_to_char_type (TREE_TYPE (stmt)) &&
          is_string_cst_addr (TREE_OPERAND (stmt, 0)))
      {
        inform (tree_nonartificial_location (stmt), "HolyC print");
      }
    }
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
