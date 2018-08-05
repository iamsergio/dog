#ifndef DOG_EXPORT_H
# define DOG_EXPORT_H

# if defined(BUILDING_DOG_PLUGIN_INTERFACE)
#  define DOG_EXPORT Q_DECL_EXPORT
# else
#  define DOG_EXPORT Q_DECL_IMPORT
# endif

#endif
