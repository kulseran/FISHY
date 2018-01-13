/**
 * BMFont XML to FontDef parser.
 */
#ifndef FISHY_BMFONTUTIL_PARSER_H
#define FISHY_BMFONTUTIL_PARSER_H

#include <CORE/BASE/status.h>
#include <CORE/UTIL/FILES/xml_parser.h>
#include <CORE/VFS/path.h>
#include <TOOLS/bmfontutil/fontdef.pb.h>

/**
 * Parse an XML Anglecode BMFont to a {@link FontDef}
 *
 * @param out the output definition
 * @param rootPath the path to the font file (used to lookup textures)
 * @param document the input document
 * @param asDistanceFont conversion option for the textures to create distance
 * fonts
 * @return true on success
 */
Status ParseDocument(
    bmfont::FontDef &out,
    const vfs::Path &rootPath,
    const core::util::files::XmlNode &document,
    const bool asDistanceFont);

#endif
