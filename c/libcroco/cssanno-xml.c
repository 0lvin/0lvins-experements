/*
  Code based on: http://lists.w3.org/Archives/Public/www-archive/2004Jun/0001.html
  
  cssanno.c -- annotate XML documents with CSS declarations

  Compile:

    gcc cssanno-xml.c `pkg-config --cflags libcroco-0.6` `pkg-config  libcroco-0.6 --libs` -o cssanno-xml

  Usage:

    cssanno-xml file.xml file.css

  cssanno takes an XML document and a CSS style sheet and
  annotates the elements with new attributes representing
  the style declarations for the current element. Example:

    <?xml version='1.0' encoding='utf-8'?>
    <foo><bar/><baz/></foo>

  with a style sheet

    foo           { display: block }
    bar           { color: red }
    baz           { font-family: "Arial Unicode MS" }
    *:first-child { white-space: normal }

  would result in an XML document like

    <?xml version="1.0" encoding="utf-8"?>
    <foo style='display: "block"; white-space: "normal";'>
           <bar style='color: "red"; white-space = "normal";' />
           <baz style='font-family: "&quot;Arial Unicode MS&quot;";' />
    </foo>

  Copyright (c) 2004 Bjoern Hoehrmann <bjoern@hoehrmann.de>.

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  $Id$

*/

#include <libcroco/libcroco.h>
#include <libxml/tree.h>
#include <libxml/xmlsave.h>
#include <assert.h>

/* namespace for css:property='...' attributes */
#define CSS_NAMESPACE "http://example.org/css#"

typedef struct
{
    xmlDoc*       document;   /* XML document */
    CRStyleSheet* stylesheet; /* style sheet */
    CRCascade*    cascade;    /* cascade abstraction */
    CRSelEng*     selector;   /* selection engine */
} workspace;

void add_single_property(xmlNode* element, CRDeclaration* decl)
{
    CRString* name = decl->property; /* name of the property */
    CRTerm* value = decl->value;     /* value of the property */

    gchar *vstr = cr_term_to_string(value); /* convert to gchar* */
    gchar *nstr = cr_string_dup2(name);     /* convert to gchar* */


    if (!vstr || !nstr)
    {
        fprintf(stderr, "%s\n", "Warning: out of memory when adding attribute");

        /* free this far allocated memory */
        if (vstr)
            g_free(vstr);

        if (nstr)
            g_free(nstr);

        return;
    }
    
    gchar *prevstyle = g_strdup("");
    
    if (xmlGetProp(element,"style") != NULL)
        prevstyle = xmlGetProp(element,"style");    
        
    gchar *style = g_strdup_printf ("%s: %s; %s", nstr, vstr, prevstyle);
    
    g_free(prevstyle);
    
    /* element.setAttributeNS(...) */
    if (!(xmlSetProp(element,"style",style)))
        fprintf(stderr, "%s\n", "Warning: Could not add attribute to element");

    g_free(style);
    
    /* free the stringified representation of the property value */
    g_free(vstr);

    /* free the stringified representation of the property name */
    g_free(nstr);
}

void add_properties(xmlNode* element, CRPropList *first)
{
    CRDeclaration *decl = NULL; /* a style declaration, a name/value pair */
    CRPropList *current = NULL; /* a list of properties */

    /* iterate over all properties in the list */
    for (current = first; current; current = cr_prop_list_get_next(current))
    {       
        decl = NULL;

        /* retrieve the declaration for the current property */
        cr_prop_list_get_decl(current, &decl);

        if (decl)
        {
            /* add the property to the element */
            add_single_property(element, decl);
        }
    }
}

void process_element(xmlNode* element, CRCascade *cascade, CRSelEng *selector)
{
    CRPropList *properties;  /* list of properties for the current element */
    xmlNode *current = NULL; /* current node */

    /* foreach element $current in $doc add properties to element */
    for (current = element; current; current = current->next)
    {
        if (current->type == XML_ELEMENT_NODE)
        {
            properties = NULL;

            /* get all properties for the current element */
            cr_sel_eng_get_matched_properties_from_cascade(selector,
                                                           cascade,
                                                           current,
                                                           &properties);

            /* add the properties as attributes to the element */
            add_properties(current, properties);
        }

        /* recursively process the children of the element */
        process_element(current->children, cascade, selector);
    }
}

void free_workspace(workspace* w)
{
    /* immediately return if there is nothing to clean up */
    if (!w)
        return;

    /* free the XML document */
    if (w->document)
        xmlFreeDoc(w->document);

    if (w->cascade)
    {
        /* free the cascade abstraction */
        cr_cascade_destroy(w->cascade);
    }
    else if (w->stylesheet)
    {
        /* free the style sheet */
        cr_stylesheet_destroy(w->stylesheet);
    }

    /* free the selector engine */
    if (w->selector)
        cr_sel_eng_destroy(w->selector);

}


void fatal_error(workspace* w, char* msg)
{
    /* free allocated workspace, if any */
    if (w)
        free_workspace(w);

    /* print an error message, if any */
    if (msg)
        fprintf(stderr, "%s\n", msg);

    /* reclaim parsing related global memory */
    xmlCleanupParser();

    /* abort */
    exit(2);
}

workspace* new_workspace()
{
    /* allocate a new workspace */
    workspace* w = (workspace*)malloc(sizeof(workspace));

    /* fatal error if malloc() fails */
    if (!w)
        fatal_error(w, "Out of memory!");

    /* initialize fields */
    w->cascade    = NULL;
    w->document   = NULL;
    w->selector   = NULL;
    w->stylesheet = NULL;

    return w;
}

int main(int argc, char** argv)
{
    enum CRStatus status  = CR_OK; /* status for libcroco operations */
    xmlNode*      node    = NULL;  /* document element */
    workspace*    w;               /* workspace */
    char*         xmlfile;         /* path to xml document */
    char*         cssfile;         /* path to style sheet */

    /* check command line parameter */
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s file.xml file.css\n", argv[0]);
        exit(1);
    }

    xmlfile = argv[1];
    cssfile = argv[2];
    
    w = new_workspace();

    /* read the XML document into memory or stop processing */
    if (!(w->document = xmlParseFile(xmlfile)))
        fatal_error(w, "Unable to process XML document.");

    /* read the style sheet into memory */
    status = cr_om_parser_simply_parse_file(cssfile, CR_ASCII, &w->stylesheet);

    /* check whether style sheet processing succeeded */
    if (status != CR_OK || w->stylesheet == NULL)
        fatal_error(w, "Unable to process style sheet.");

    /* retrieve a pointer to the document element */
    node = xmlDocGetRootElement(w->document);

    /* at this point, the document must have a root element */
    assert( node != NULL );

    /* new cascade abstraction */
    if (!(w->cascade = cr_cascade_new(w->stylesheet, NULL, NULL)))
        fatal_error(w, "Unable to create cascade abstraction");

    /* new selector engine */
    if (!(w->selector = cr_sel_eng_new()))
        fatal_error(w, "Unable to create selector engine.");

    /* traverse the tree */
    process_element(node, w->cascade, w->selector);

    /* write the document to stdout */
    if (xmlSaveFile("-", w->document) == -1)
        fatal_error(w, "Could not write XML document to stdout!");

    /* free workspace resources */
    free_workspace(w);

    /* reclaim parsing related global memory */
    xmlCleanupParser();

    return EXIT_SUCCESS;
}

