/* -*- mode: C; c-basic-offset: 4; indent-tabs-mode: nil; -*- */
/* vim:set et sts=4: */
/* IBus - The Input Bus
 * Copyright (C) 2008-2010 Peng Huang <shawn.p.huang@gmail.com>
 * Copyright (C) 2008-2010 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#include "ibusproperty.h"
#include "ibusproplist.h"

/* functions prototype */
static void         ibus_property_destroy       (IBusProperty       *prop);
static gboolean     ibus_property_serialize     (IBusProperty       *prop,
                                                 GVariantBuilder    *builder);
static gint         ibus_property_deserialize   (IBusProperty       *prop,
                                                 GVariant           *variant);
static gboolean     ibus_property_copy          (IBusProperty       *dest,
                                                 const IBusProperty *src);

G_DEFINE_TYPE (IBusProperty, ibus_property, IBUS_TYPE_SERIALIZABLE)

static void
ibus_property_class_init (IBusPropertyClass *class)
{
    IBusObjectClass *object_class = IBUS_OBJECT_CLASS (class);
    IBusSerializableClass *serializable_class = IBUS_SERIALIZABLE_CLASS (class);

    object_class->destroy = (IBusObjectDestroyFunc) ibus_property_destroy;

    serializable_class->serialize   = (IBusSerializableSerializeFunc) ibus_property_serialize;
    serializable_class->deserialize = (IBusSerializableDeserializeFunc) ibus_property_deserialize;
    serializable_class->copy        = (IBusSerializableCopyFunc) ibus_property_copy;
}

static void
ibus_property_init (IBusProperty *prop)
{
    prop->key = NULL;
    prop->type = 0;
    prop->label = NULL;
    prop->icon = NULL;
    prop->tooltip = NULL;
    prop->sensitive = FALSE;
    prop->visible = FALSE;
    prop->state = 0;

    prop->sub_props = NULL;
}

static void
ibus_property_destroy (IBusProperty *prop)
{
    g_free (prop->key);
    prop->key = NULL;

    g_free (prop->icon);
    prop->icon = NULL;

    if (prop->label) {
        g_object_unref (prop->label);
        prop->label = NULL;
    }

    if (prop->tooltip) {
        g_object_unref (prop->tooltip);
        prop->tooltip = NULL;
    }

    if (prop->sub_props) {
        g_object_unref (prop->sub_props);
        prop->sub_props = NULL;
    }

    IBUS_OBJECT_CLASS (ibus_property_parent_class)->destroy ((IBusObject *)prop);
}

gboolean
ibus_property_serialize (IBusProperty    *prop,
                         GVariantBuilder *builder)
{
    gboolean retval;

    retval = IBUS_SERIALIZABLE_CLASS (ibus_property_parent_class)->serialize ((IBusSerializable *) prop, builder);
    g_return_val_if_fail (retval, FALSE);

    g_return_val_if_fail (IBUS_IS_PROPERTY (prop), FALSE);

    g_variant_builder_add (builder, "s", prop->key);
    g_variant_builder_add (builder, "u", prop->type);
    g_variant_builder_add (builder, "v", ibus_serializable_serialize ((IBusSerializable *)prop->label));
    g_variant_builder_add (builder, "s", prop->icon);
    g_variant_builder_add (builder, "v", ibus_serializable_serialize ((IBusSerializable *)prop->tooltip));
    g_variant_builder_add (builder, "b", prop->sensitive);
    g_variant_builder_add (builder, "b", prop->visible);
    g_variant_builder_add (builder, "u", prop->state);
    g_variant_builder_add (builder, "v", ibus_serializable_serialize ((IBusSerializable *)prop->sub_props));

    return TRUE;
}

static gint
ibus_property_deserialize (IBusProperty *prop,
                           GVariant     *variant)
{
    gint retval;

    retval = IBUS_SERIALIZABLE_CLASS (ibus_property_parent_class)->deserialize ((IBusSerializable *) prop, variant);
    g_return_val_if_fail (retval, 0);

    g_variant_get_child (variant, retval++, "s", &prop->key);
    g_variant_get_child (variant, retval++, "u", &prop->type);

    GVariant *subvar = g_variant_get_child_value (variant, retval++);
    prop->label = IBUS_TEXT (ibus_serializable_deserialize (subvar));
    g_object_ref_sink (prop->label);
    g_variant_unref (subvar);

    g_variant_get_child (variant, retval++, "s", &prop->icon);

    subvar = g_variant_get_child_value (variant, retval++);
    prop->tooltip = IBUS_TEXT (ibus_serializable_deserialize (subvar));
    g_object_ref_sink (prop->tooltip);
    g_variant_unref (subvar);

    g_variant_get_child (variant, retval++, "b", &prop->sensitive);
    g_variant_get_child (variant, retval++, "b", &prop->visible);
    g_variant_get_child (variant, retval++, "u", &prop->state);

    subvar = g_variant_get_child_value (variant, retval++);
    prop->sub_props = IBUS_PROP_LIST (ibus_serializable_deserialize (subvar));
    g_object_ref_sink (prop->sub_props);
    g_variant_unref (subvar);

    return retval;
}

static gboolean
ibus_property_copy (IBusProperty       *dest,
                    const IBusProperty *src)
{
    gboolean retval;

    retval = IBUS_SERIALIZABLE_CLASS (ibus_property_parent_class)->copy ((IBusSerializable *) dest, (IBusSerializable *) src);
    g_return_val_if_fail (retval, FALSE);

    g_return_val_if_fail (IBUS_IS_PROPERTY (dest), FALSE);
    g_return_val_if_fail (IBUS_IS_PROPERTY (src), FALSE);

    dest->key = g_strdup (src->key);
    dest->icon = g_strdup (src->icon);
    if (src->label) {
        dest->label = (IBusText *) ibus_serializable_copy ((IBusSerializable *) src->label);
    }
    else
        dest->label = ibus_text_new_from_static_string ("");
    if (src->tooltip) {
        dest->tooltip = (IBusText *) ibus_serializable_copy ((IBusSerializable *) src->tooltip);
    }
    else
        dest->tooltip = ibus_text_new_from_static_string ("");

    dest->sensitive = src->sensitive;
    dest->visible = src->visible;
    dest->type = src->type;
    dest->state = src->state;

    dest->sub_props = (IBusPropList *) ibus_serializable_copy ((IBusSerializable *) src->sub_props);

    return TRUE;
}

IBusProperty *
ibus_property_new (const gchar   *key,
                   IBusPropType   type,
                   IBusText      *label,
                   const gchar   *icon,
                   IBusText      *tooltip,
                   gboolean       sensitive,
                   gboolean       visible,
                   IBusPropState  state,
                   IBusPropList  *prop_list)
{
    g_return_val_if_fail (key != NULL, NULL);
    g_return_val_if_fail (type >= PROP_TYPE_NORMAL &&
                          type <= PROP_TYPE_SEPARATOR,
                          NULL);

    IBusProperty *prop;

    prop = (IBusProperty *)g_object_new (IBUS_TYPE_PROPERTY, NULL);

    prop->key = g_strdup (key);
    prop->type = type;

    ibus_property_set_label (prop, label);
    ibus_property_set_icon (prop, icon);
    ibus_property_set_tooltip (prop, tooltip);
    ibus_property_set_sensitive (prop, sensitive);
    ibus_property_set_visible (prop, visible);
    ibus_property_set_state (prop, state);
    ibus_property_set_sub_props (prop, prop_list);

    return prop;
}

#define IBUS_PROPERTY_GET_FIELD(field, return_type) \
return_type                                                             \
ibus_property_get_ ## field (IBusProperty *prop)                        \
{                                                                       \
    return prop->field;                                                 \
}

IBUS_PROPERTY_GET_FIELD (key, const gchar *)
IBUS_PROPERTY_GET_FIELD (icon, const gchar *)
IBUS_PROPERTY_GET_FIELD (label, const IBusText *)
IBUS_PROPERTY_GET_FIELD (tooltip, const IBusText *)
IBUS_PROPERTY_GET_FIELD (sensitive, gboolean)
IBUS_PROPERTY_GET_FIELD (visible, gboolean)
IBUS_PROPERTY_GET_FIELD (state, IBusPropState)
IBUS_PROPERTY_GET_FIELD (sub_props, const IBusPropList *)
#undef IBUS_PROPERTY_GET_FIELD

/* ibus_property_get_type() exists */
IBusPropType
ibus_property_get_prop_type (IBusProperty *prop)
{
    return prop->type;
}

void
ibus_property_set_label (IBusProperty *prop,
                         IBusText     *label)
{
    g_assert (IBUS_IS_PROPERTY (prop));
    g_return_if_fail (label == NULL || IBUS_IS_TEXT (label));

    if (prop->label) {
        g_object_unref (prop->label);
    }

    if (label == NULL) {
        prop->label = ibus_text_new_from_static_string ("");
    }
    else {
        prop->label = g_object_ref_sink (label);
    }
}

void
ibus_property_set_icon (IBusProperty *prop,
                        const gchar  *icon)
{
    g_assert (IBUS_IS_PROPERTY (prop));

    g_free (prop->icon);
    prop->icon = g_strdup (icon != NULL ? icon : "");
}

void
ibus_property_set_tooltip (IBusProperty *prop,
                           IBusText     *tooltip)
{
    g_assert (IBUS_IS_PROPERTY (prop));
    g_assert (tooltip == NULL || IBUS_IS_TEXT (tooltip));

    if (prop->tooltip) {
        g_object_unref (prop->tooltip);
    }

    if (tooltip == NULL) {
        prop->tooltip = ibus_text_new_from_static_string ("");
        g_object_ref_sink (prop->tooltip);
    }
    else {
        prop->tooltip = tooltip;
        g_object_ref_sink (prop->tooltip);
    }
}

void
ibus_property_set_sensitive (IBusProperty *prop,
                             gboolean      sensitive)
{
    g_assert (IBUS_IS_PROPERTY (prop));
    prop->sensitive = sensitive;
}

void
ibus_property_set_visible (IBusProperty *prop,
                           gboolean      visible)
{
    g_assert (IBUS_IS_PROPERTY (prop));
    prop->visible = visible;
}

void
ibus_property_set_state (IBusProperty  *prop,
                         IBusPropState  state)
{
    g_assert (IBUS_IS_PROPERTY (prop));
    g_assert (state == PROP_STATE_UNCHECKED ||
              state == PROP_STATE_CHECKED ||
              state == PROP_STATE_INCONSISTENT);

    prop->state = state;
}

void
ibus_property_set_sub_props (IBusProperty *prop,
                             IBusPropList *prop_list)
{
    g_assert (IBUS_IS_PROPERTY (prop));
    g_assert (IBUS_IS_PROP_LIST (prop_list) || prop_list == NULL);

    if (prop->sub_props) {
        g_object_unref (prop->sub_props);
    }

    if (prop_list) {
        prop->sub_props = prop_list;
        g_object_ref_sink (prop_list);
    }
    else {
        prop->sub_props = ibus_prop_list_new ();
        g_object_ref_sink (prop->sub_props);
    }
}

gboolean
ibus_property_update (IBusProperty *prop,
                      IBusProperty *prop_update)
{
    g_assert (IBUS_IS_PROPERTY (prop));
    g_assert (IBUS_IS_PROPERTY (prop_update));

    if (g_strcmp0 (prop->key, prop_update->key) != 0) {
        return ibus_prop_list_update_property (prop->sub_props, prop_update);
    }

    g_free (prop->icon);
    prop->icon = g_strdup (prop_update->icon);

    if (prop->label) {
        g_object_unref (prop->label);
    }
    prop->label = (IBusText *) g_object_ref_sink (prop_update->label);

    if (prop->tooltip) {
        g_object_unref (prop->tooltip);
    }
    prop->tooltip = (IBusText *) g_object_ref_sink (prop_update->tooltip);
    prop->visible = prop_update->visible;
    prop->state = prop_update->state;
    prop->sensitive = prop_update->sensitive;

    return TRUE;
}


