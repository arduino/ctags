#include "general.h"	/* must always come first */
#include "debug.h"
#include "entry.h"
#include "keyword.h"
#include "parse.h"
#include "read.h"
#include "routines.h"
#include "string.h"
#include "vstring.h"
#include <glib.h>
#include <assert.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "ctags-utils.h"

static kindOption Kinds [] = {
	{ TRUE,  'f', "function",	  "functions"},
	{ TRUE,  'c', "class",		  "classes"},
	{ TRUE,  'm', "method",		  "methods"},
	{ TRUE,  'p', "property",	  "properties"},
	{ TRUE,  'v', "variable",	  "global variables"}
};

static void
initialize (const langType language)
{
}

static void
parse_function (xmlNode *node, const gchar *parent)
{
	xmlNode *i, *k;
	gchar *name;
	tagEntryInfo *tag;

	g_assert (node != NULL);

	name = (gchar*)xmlGetProp (node, (xmlChar*)"name");
	if (!name)
		return;

	tag = (tagEntryInfo*)malloc (sizeof (tagEntryInfo));
	initTagEntry (tag, name);
	get_file_pos (node->line, &tag->filePosition, File.fp);
	tag->lineNumber = node->line;
	tag->isFileScope	= 1;
	tag->kindName = "function";
	tag->kind = 'f';
	if (parent) {
		tag->kindName = "member";
		tag->kind = 'm';
		tag->extensionFields.scope[0] = "class";
		tag->extensionFields.scope[1] = parent;
	}

	for (i = node->children; i; i = i->next)
	{
		if (!i->name)
			continue;
		if (strcmp ((const gchar*)i->name, "return-value") == 0)
		{
			for (k = i->children; k; k = k->next)
			{
				const gchar *tmp;
				if (!k->name)
					continue;
				tmp = (const gchar*)xmlGetProp (k, (const xmlChar*)"name");
				if (!tmp)
					continue;
				tag->extensionFields.returnType = tmp;
			}
		}
		if (strcmp ((const gchar*)i->name, "parameters") == 0)
		{
			for (k = i->children; k; k = k->next)
			{
/*TODO:				const gchar *name;
				if (!k->name)
					continue;
				name = (const gchar*)xmlGetProp (node, (const xmlChar*)"name");
				if (!name)
					continue;
				tmp = g_new (Argument, 1);
				tmp->name = g_strdup (name);
				tmp->types = NULL;
				ret->args = g_list_append (ret->args, tmp);*/
			}
		}
	}
	makeTagEntry (tag);
}

static void makeTags (xmlNode *node, const gchar *parent);

static void
parse_class (xmlNode *node)
{
	xmlNode *i;
	gchar *name;

	g_assert (node);

	name = (gchar*)xmlGetProp (node, (const xmlChar*)"name");
	if (!name)
		return;

	tagEntryInfo *tag = (tagEntryInfo*)malloc (sizeof (tagEntryInfo));
	initTagEntry (tag, name);
	tag->isFileScope = 1;
	tag->kindName = "class";
	tag->kind = 'c';
	get_file_pos (node->line, &tag->filePosition, File.fp);
	tag->lineNumber = node->line;
	makeTagEntry (tag);

	for (i = node->children; i; i = i->next)
	{
		makeTags (i, name);
	}
}

static void
makeTags (xmlNode *node, const gchar *parent)
{
	g_assert (node != NULL);
	g_assert (node->name != NULL);

	if (strcmp ((const gchar*)node->name, "text") == 0
			|| strcmp ((const gchar*)node->name, "implements") == 0)
		return;
	if (strcmp ((const gchar*)node->name, "enumeration") == 0
			|| strcmp ((const gchar*)node->name, "union") == 0
			|| strcmp ((const gchar*)node->name, "namespace") == 0
			|| strcmp ((const gchar*)node->name, "class") == 0
			|| strcmp ((const gchar*)node->name, "record") == 0
			|| strcmp ((const gchar*)node->name, "bitfield") == 0
			|| strcmp ((const gchar*)node->name, "interface") == 0)
	{
		parse_class (node);
		return;
	}
	if (strcmp ((const gchar*)node->name, "function") == 0 || strcmp ((const gchar*)node->name, "method") == 0
			|| strcmp ((const gchar*)node->name, "callback") == 0
			|| strcmp ((const gchar*)node->name, "constructor") == 0)
	{
		parse_function (node, parent);
		return;
	}
	if (strcmp ((const gchar*)node->name, "alias") == 0 ||
			strcmp ((const gchar*)node->name, "constant") == 0 ||
			strcmp ((const gchar*)node->name, "signal") == 0 ||
			strcmp ((const gchar*)node->name, "field") == 0 ||
			strcmp ((const gchar*)node->name, "property") == 0 ||
			strcmp ((const gchar*)node->name, "member") == 0)
	{
		gchar *name = (gchar*)xmlGetProp (node, (const xmlChar*)"name");
		if (!name)
			return;
		tagEntryInfo *tag = (tagEntryInfo*)malloc (sizeof (tagEntryInfo));
		initTagEntry (tag, name);
		tag->isFileScope	= 1;
		tag->kindName = "variable";
		tag->kind = 'v';
		get_file_pos (node->line, &tag->filePosition, File.fp);
		tag->lineNumber = node->line;
		if (parent) {
			tag->kindName = "member";
			tag->kind = 'm';
			tag->extensionFields.scope[0] = "class";
			tag->extensionFields.scope[1] = parent;
		}
		makeTagEntry (tag);
		return;
	}
}

static void
findTags (void)
{
	xmlNode *i;
	xmlDocPtr doc = xmlParseFile(getInputFileName());
	xmlNode *root;

	if (doc == NULL) {
		g_warning ("could not parse file");
	}
	root = xmlDocGetRootElement(doc);
	for (i = root->children; i; i = i->next)
	{
		xmlNode *j;
		if (!i->name)
			continue;
		if (strcmp ((const char*)i->name, "namespace") !=0)
			continue;
		for (j = i->children; j; j = j->next)
		{
			makeTags (j, NULL);
		}
	}
}

extern parserDefinition*
GirParser (void)
{
	static const char *const extensions [] = { "gir", NULL };
	parserDefinition *const def = parserNew ("GObject-Introspection");
	def->extensions = extensions;

	def->kinds = Kinds;
	def->kindCount = KIND_COUNT (Kinds);
	def->parser = findTags;
	def->initialize = initialize;

	return def;
}
