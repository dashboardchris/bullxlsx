#include <array>
#include <sstream>
#include <string>
#include <unordered_map>

#include <pugixml.hpp>

#include "writer/writer.hpp"
#include "cell/cell.hpp"
#include "constants.hpp"
#include "worksheet/range.hpp"
#include "worksheet/range_reference.hpp"
#include "worksheet/worksheet.hpp"
#include "workbook/workbook.hpp"

namespace xlnt {

std::string writer::write_string_table(const std::vector<std::string> &string_table)
{
    pugi::xml_document doc;
    auto root_node = doc.append_child("sst");
    root_node.append_attribute("xmlns").set_value("http://schemas.openxmlformats.org/spreadsheetml/2006/main");
    root_node.append_attribute("uniqueCount").set_value((int)string_table.size());
    
    for(auto string : string_table)
    {
        root_node.append_child("si").append_child("t").text().set(string.c_str());
    }
    
    std::stringstream ss;
    doc.save(ss);
    
    return ss.str();
}

std::string writer::write_workbook_rels(const workbook &/*wb*/)
{
    static const std::vector<std::pair<std::string, std::pair<std::string, std::string>>> rels =
    {
        {"rId1", {"worksheets/sheet1.xml", "http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet"}},
        {"rId2", {"sharedStrings.xml", "http://schemas.openxmlformats.org/officeDocument/2006/relationships/sharedStrings"}},
        {"rId3", {"styles.xml", "http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles"}},
        {"rId4", {"theme/theme1.xml", "http://schemas.openxmlformats.org/officeDocument/2006/relationships/theme"}}
    };
    
    return write_relationships(rels);
}

std::string writer::write_worksheet_rels(worksheet /*ws*/, int)
{
  return "";
}


std::string writer::write_workbook(const workbook &wb)
{
    pugi::xml_document doc;
    auto root_node = doc.append_child("workbook");
    root_node.append_attribute("xmlns").set_value("http://schemas.openxmlformats.org/spreadsheetml/2006/main");
    root_node.append_attribute("xmlns:r").set_value("http://schemas.openxmlformats.org/officeDocument/2006/relationships");
    
    auto file_version_node = root_node.append_child("fileVersion");
    file_version_node.append_attribute("appName").set_value("xl");
    file_version_node.append_attribute("lastEdited").set_value("4");
    file_version_node.append_attribute("lowestEdited").set_value("4");
    file_version_node.append_attribute("rupBuild").set_value("4505");
    
    auto workbook_pr_node = root_node.append_child("workbookPr");
    workbook_pr_node.append_attribute("codeName").set_value("ThisWorkbook");
    workbook_pr_node.append_attribute("defaultThemeVersion").set_value("124226");
    
    auto book_views_node = root_node.append_child("bookViews");
    auto workbook_view_node = book_views_node.append_child("workbookView");
    workbook_view_node.append_attribute("activeTab").set_value("0");
    workbook_view_node.append_attribute("autoFilterDateGrouping").set_value("1");
    workbook_view_node.append_attribute("firstSheet").set_value("0");
    workbook_view_node.append_attribute("minimized").set_value("0");
    workbook_view_node.append_attribute("showHorizontalScroll").set_value("1");
    workbook_view_node.append_attribute("showSheetTabs").set_value("1");
    workbook_view_node.append_attribute("showVerticalScroll").set_value("1");
    workbook_view_node.append_attribute("tabRatio").set_value("600");
    workbook_view_node.append_attribute("visibility").set_value("visible");
    
    auto sheets_node = root_node.append_child("sheets");
    
    int i = 0;
    for(auto ws : wb)
    {
        auto sheet_node = sheets_node.append_child("sheet");
        sheet_node.append_attribute("name").set_value(ws.get_title().c_str());
        sheet_node.append_attribute("r:id").set_value((std::string("rId") + std::to_string(i + 1)).c_str());
        sheet_node.append_attribute("sheetId").set_value(std::to_string(i + 1).c_str());
        i++;
    }
    
    root_node.append_child("definedNames");
    
    auto calc_pr_node = root_node.append_child("calcPr");
    calc_pr_node.append_attribute("calcId").set_value("124519");
    calc_pr_node.append_attribute("calcMode").set_value("auto");
    calc_pr_node.append_attribute("fullCalcOnLoad").set_value("1");
    
    std::stringstream ss;
    doc.save(ss);
    
    return ss.str();
}

std::string writer::write_worksheet(worksheet ws, const std::vector<std::string> &string_table)
{
  return write_worksheet(ws, string_table, {});
}

std::string writer::write_worksheet(worksheet ws, const std::vector<std::string> &string_table, const std::unordered_map<std::size_t, std::string> &)
{
    pugi::xml_document doc;
    auto root_node = doc.append_child("worksheet");
    root_node.append_attribute("xmlns").set_value(constants::Namespaces.at("spreadsheetml").c_str());
    root_node.append_attribute("xmlns:r").set_value(constants::Namespaces.at("r").c_str());
    auto sheet_pr_node = root_node.append_child("sheetPr");
    auto outline_pr_node = sheet_pr_node.append_child("outlinePr");
    outline_pr_node.append_attribute("summaryBelow").set_value(1);
    outline_pr_node.append_attribute("summaryRight").set_value(1);
    auto dimension_node = root_node.append_child("dimension");
    dimension_node.append_attribute("ref").set_value(ws.calculate_dimension().to_string().c_str());
    auto sheet_views_node = root_node.append_child("sheetViews");
    auto sheet_view_node = sheet_views_node.append_child("sheetView");
    sheet_view_node.append_attribute("workbookViewId").set_value(0);
    
    auto selection_node = sheet_view_node.append_child("selection");
    
    std::string active_cell = "A1";
    selection_node.append_attribute("activeCell").set_value(active_cell.c_str());
    selection_node.append_attribute("sqref").set_value(active_cell.c_str());
    
    auto sheet_format_pr_node = root_node.append_child("sheetFormatPr");
    sheet_format_pr_node.append_attribute("baseColWidth").set_value(10);
    sheet_format_pr_node.append_attribute("defaultRowHeight").set_value(15);
    
    auto sheet_data_node = root_node.append_child("sheetData");
    for(auto row : ws.rows())
    {
        row_t min = (int)row.num_cells();
        row_t max = 0;
        bool any_non_null = false;
        
        for(auto cell : row)
        {
            min = std::min(min, cell_reference::column_index_from_string(cell.get_column()));
            max = std::max(max, cell_reference::column_index_from_string(cell.get_column()));
            
            if(cell.get_data_type() != cell::type::null)
            {
                any_non_null = true;
            }
        }
        
        if(!any_non_null)
        {
            continue;
        }
        
        auto row_node = sheet_data_node.append_child("row");
        row_node.append_attribute("r").set_value(row.front().get_row());
        
        row_node.append_attribute("spans").set_value((std::to_string(min) + ":" + std::to_string(max)).c_str());
        //row_node.append_attribute("x14ac:dyDescent").set_value(0.25);
        
        for(auto cell : row)
        {
            if(cell.get_data_type() != cell::type::null || cell.is_merged())
            {
                auto cell_node = row_node.append_child("c");
                cell_node.append_attribute("r").set_value(cell.get_reference().to_string().c_str());
                
                if(cell.get_data_type() == cell::type::string)
                {
                    int match_index = -1;
                    for(int i = 0; i < (int)string_table.size(); i++)
                    {
                        if(string_table[i] == cell.get_value())
                        {
                            match_index = i;
                            break;
                        }
                    }
                    
                    if(match_index == -1)
                    {
                        cell_node.append_attribute("t").set_value("inlineStr");
                        auto inline_string_node = cell_node.append_child("is");
                        inline_string_node.append_child("t").text().set(cell.get_value().c_str());
                    }
                    else
                    {
                        cell_node.append_attribute("t").set_value("s");
                        auto value_node = cell_node.append_child("v");
                        value_node.text().set(match_index);
                    }
                }
                else
                {
                    if(cell.get_data_type() != cell::type::null)
                    {
                        if(cell.get_data_type() == cell::type::boolean)
                        {
                            cell_node.append_attribute("t").set_value("b");
                            auto value_node = cell_node.append_child("v");
                            value_node.text().set(cell.get_value().c_str());
                        }
                        else if(cell.get_data_type() == cell::type::numeric)
                        {
                            cell_node.append_attribute("t").set_value("n");
                            auto value_node = cell_node.append_child("v");
                            value_node.text().set(cell.get_value().c_str());
                        }
                        else if(cell.get_data_type() == cell::type::formula)
                        {
                            cell_node.append_child("f").text().set(cell.get_value().substr(1).c_str());
                            cell_node.append_child("v");
                        }
                    }
                }
            }
        }
    }
    
    if(!ws.get_merged_ranges().empty())
    {
        auto merge_cells_node = root_node.append_child("mergeCells");
        merge_cells_node.append_attribute("count").set_value((unsigned int)ws.get_merged_ranges().size());
        
        for(auto merged_range : ws.get_merged_ranges())
        {
            auto merge_cell_node = merge_cells_node.append_child("mergeCell");
            merge_cell_node.append_attribute("ref").set_value(merged_range.to_string().c_str());
        }
    }
    
    auto page_margins_node = root_node.append_child("pageMargins");
        
    page_margins_node.append_attribute("left").set_value(ws.get_page_margins().get_left());
    page_margins_node.append_attribute("right").set_value(ws.get_page_margins().get_right());
    page_margins_node.append_attribute("top").set_value(ws.get_page_margins().get_top());
    page_margins_node.append_attribute("bottom").set_value(ws.get_page_margins().get_bottom());
    page_margins_node.append_attribute("header").set_value(ws.get_page_margins().get_header());
    page_margins_node.append_attribute("footer").set_value(ws.get_page_margins().get_footer());
    
    if(!ws.get_page_setup().is_default())
    {
        auto page_setup_node = root_node.append_child("pageSetup");
        
        std::string orientation_string = ws.get_page_setup().get_orientation() == page_setup::orientation::landscape ? "landscape" : "portrait";
        page_setup_node.append_attribute("orientation").set_value(orientation_string.c_str());
        page_setup_node.append_attribute("paperSize").set_value((int)ws.get_page_setup().get_paper_size());
        page_setup_node.append_attribute("fitToHeight").set_value(ws.get_page_setup().fit_to_height() ? 1 : 0);
        page_setup_node.append_attribute("fitToWidth").set_value(ws.get_page_setup().fit_to_width() ? 1 : 0);
        
        auto page_set_up_pr_node = root_node.append_child("pageSetUpPr");
        page_set_up_pr_node.append_attribute("fitToPage").set_value(ws.get_page_setup().fit_to_page() ? 1 : 0);
    }
    
    std::stringstream ss;
    doc.save(ss);
    
    return ss.str();
}

std::string writer::write_worksheet(worksheet ws)
{
    return write_worksheet(ws, std::vector<std::string>());
}

    
std::string writer::write_content_types(const std::pair<std::unordered_map<std::string, std::string>, std::unordered_map<std::string, std::string>> &content_types)
{
    /*std::set<std::string> seen;
     
     pugi::xml_node root;
     
     if(wb.has_vba_archive())
     {
     root = fromstring(wb.get_vba_archive().read(ARC_CONTENT_TYPES));
     
     for(auto elem : root.findall("{" + CONTYPES_NS + "}Override"))
     {
     seen.insert(elem.attrib["PartName"]);
     }
     }
     else
     {
     root = Element("{" + CONTYPES_NS + "}Types");
     
     for(auto content_type : static_content_types_config)
     {
     if(setting_type == "Override")
     {
     tag = "{" + CONTYPES_NS + "}Override";
     attrib = {"PartName": "/" + name};
     }
     else
     {
     tag = "{" + CONTYPES_NS + "}Default";
     attrib = {"Extension": name};
     }
     
     attrib["ContentType"] = content_type;
     SubElement(root, tag, attrib);
     }
     }
     
     int drawing_id = 1;
     int chart_id = 1;
     int comments_id = 1;
     int sheet_id = 0;
     
     for(auto sheet : wb)
     {
     std::string name = "/xl/worksheets/sheet" + std::to_string(sheet_id) + ".xml";
     
     if(seen.find(name) == seen.end())
     {
     SubElement(root, "{" + CONTYPES_NS + "}Override", {{"PartName", name},
     {"ContentType", "application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml"}});
     }
     
     if(sheet._charts || sheet._images)
     {
     name = "/xl/drawings/drawing" + drawing_id + ".xml";
     
     if(seen.find(name) == seen.end())
     {
     SubElement(root, "{%s}Override" % CONTYPES_NS, {"PartName" : name,
     "ContentType" : "application/vnd.openxmlformats-officedocument.drawing+xml"});
     }
     
     drawing_id += 1;
     
     for(auto chart : sheet._charts)
     {
     name = "/xl/charts/chart%d.xml" % chart_id;
     
     if(seen.find(name) == seen.end())
     {
     SubElement(root, "{%s}Override" % CONTYPES_NS, {"PartName" : name,
     "ContentType" : "application/vnd.openxmlformats-officedocument.drawingml.chart+xml"});
     }
     
     chart_id += 1;
     
     if(chart._shapes)
     {
     name = "/xl/drawings/drawing%d.xml" % drawing_id;
     
     if(seen.find(name) == seen.end())
     {
     SubElement(root, "{%s}Override" % CONTYPES_NS, {"PartName" : name,
     "ContentType" : "application/vnd.openxmlformats-officedocument.drawingml.chartshapes+xml"});
     }
     
     drawing_id += 1;
     }
     }
     }
     if(sheet.get_comment_count() > 0)
     {
     SubElement(root, "{%s}Override" % CONTYPES_NS,
     {"PartName": "/xl/comments%d.xml" % comments_id,
     "ContentType" : "application/vnd.openxmlformats-officedocument.spreadsheetml.comments+xml"});
     comments_id += 1;
     }
     }*/
    
    pugi::xml_document doc;
    auto root_node = doc.append_child("Types");
    root_node.append_attribute("xmlns").set_value(constants::Namespaces.at("content-types").c_str());
    
    for(auto default_type : content_types.first)
    {
        auto xml_node = root_node.append_child("Default");
        xml_node.append_attribute("Extension").set_value(default_type.first.c_str());
        xml_node.append_attribute("ContentType").set_value(default_type.second.c_str());
    }
    
    for(auto override_type : content_types.second)
    {
        auto theme_node = root_node.append_child("Override");
        theme_node.append_attribute("PartName").set_value(override_type.first.c_str());
        theme_node.append_attribute("ContentType").set_value(override_type.second.c_str());
    }
    
    std::stringstream ss;
    doc.save(ss);
    return ss.str();
}

std::string writer::write_relationships(const std::vector<std::pair<std::string, std::pair<std::string, std::string>>> &relationships)
{
    pugi::xml_document doc;
    auto root_node = doc.append_child("Relationships");
    root_node.append_attribute("xmlns").set_value(constants::Namespaces.at("relationships").c_str());
    
    for(auto relationship : relationships)
    {
        auto app_props_node = root_node.append_child("Relationship");
        app_props_node.append_attribute("Id").set_value(relationship.first.c_str());
        app_props_node.append_attribute("Target").set_value(relationship.second.first.c_str());
        app_props_node.append_attribute("Type").set_value(relationship.second.second.c_str());
    }
    
    std::stringstream ss;
    doc.save(ss);
    return ss.str();
}
    
std::string writer::write_theme()
{
    /*
     pugi::xml_document doc;
     auto theme_node = doc.append_child("a:theme");
     theme_node.append_attribute("xmlns:a").set_value(constants::Namespaces.at("drawingml").c_str());
     theme_node.append_attribute("name").set_value("Office Theme");
     auto theme_elements_node = theme_node.append_child("a:themeElements");
     auto clr_scheme_node = theme_elements_node.append_child("a:clrScheme");
     clr_scheme_node.append_attribute("name").set_value("Office");
     
     struct scheme_element
     {
     std::string name;
     std::string sub_element_name;
     std::string val;
     };
     
     std::vector<scheme_element> scheme_elements =
     {
     {"a:dk1", "a:sysClr", "windowText"}
     };
     
     for(auto element : scheme_elements)
     {
     auto element_node = clr_scheme_node.append_child("a:dk1");
     element_node.append_child(element.sub_element_name.c_str()).append_attribute("val").set_value(element.val.c_str());
     }
     
     std::stringstream ss;
     doc.print(ss);
     return ss.str();
     */
    std::array<unsigned char, 6994> data =
    {{
        60, 63, 120, 109, 108, 32, 118, 101, 114, 115,
        105, 111, 110, 61, 34, 49, 46, 48, 34, 32,
        101, 110, 99, 111, 100, 105, 110, 103, 61, 34,
        85, 84, 70, 45, 56, 34, 32, 115, 116, 97,
        110, 100, 97, 108, 111, 110, 101, 61, 34, 121,
        101, 115, 34, 63, 62, 10, 60, 97, 58, 116,
        104, 101, 109, 101, 32, 120, 109, 108, 110, 115,
        58, 97, 61, 34, 104, 116, 116, 112, 58, 47,
        47, 115, 99, 104, 101, 109, 97, 115, 46, 111,
        112, 101, 110, 120, 109, 108, 102, 111, 114, 109,
        97, 116, 115, 46, 111, 114, 103, 47, 100, 114,
        97, 119, 105, 110, 103, 109, 108, 47, 50, 48,
        48, 54, 47, 109, 97, 105, 110, 34, 32, 110,
        97, 109, 101, 61, 34, 79, 102, 102, 105, 99,
        101, 32, 84, 104, 101, 109, 101, 34, 62, 60,
        97, 58, 116, 104, 101, 109, 101, 69, 108, 101,
        109, 101, 110, 116, 115, 62, 60, 97, 58, 99,
        108, 114, 83, 99, 104, 101, 109, 101, 32, 110,
        97, 109, 101, 61, 34, 79, 102, 102, 105, 99,
        101, 34, 62, 60, 97, 58, 100, 107, 49, 62,
        60, 97, 58, 115, 121, 115, 67, 108, 114, 32,
        118, 97, 108, 61, 34, 119, 105, 110, 100, 111,
        119, 84, 101, 120, 116, 34, 32, 108, 97, 115,
        116, 67, 108, 114, 61, 34, 48, 48, 48, 48,
        48, 48, 34, 47, 62, 60, 47, 97, 58, 100,
        107, 49, 62, 60, 97, 58, 108, 116, 49, 62,
        60, 97, 58, 115, 121, 115, 67, 108, 114, 32,
        118, 97, 108, 61, 34, 119, 105, 110, 100, 111,
        119, 34, 32, 108, 97, 115, 116, 67, 108, 114,
        61, 34, 70, 70, 70, 70, 70, 70, 34, 47,
        62, 60, 47, 97, 58, 108, 116, 49, 62, 60,
        97, 58, 100, 107, 50, 62, 60, 97, 58, 115,
        114, 103, 98, 67, 108, 114, 32, 118, 97, 108,
        61, 34, 49, 70, 52, 57, 55, 68, 34, 47,
        62, 60, 47, 97, 58, 100, 107, 50, 62, 60,
        97, 58, 108, 116, 50, 62, 60, 97, 58, 115,
        114, 103, 98, 67, 108, 114, 32, 118, 97, 108,
        61, 34, 69, 69, 69, 67, 69, 49, 34, 47,
        62, 60, 47, 97, 58, 108, 116, 50, 62, 60,
        97, 58, 97, 99, 99, 101, 110, 116, 49, 62,
        60, 97, 58, 115, 114, 103, 98, 67, 108, 114,
        32, 118, 97, 108, 61, 34, 52, 70, 56, 49,
        66, 68, 34, 47, 62, 60, 47, 97, 58, 97,
        99, 99, 101, 110, 116, 49, 62, 60, 97, 58,
        97, 99, 99, 101, 110, 116, 50, 62, 60, 97,
        58, 115, 114, 103, 98, 67, 108, 114, 32, 118,
        97, 108, 61, 34, 67, 48, 53, 48, 52, 68,
        34, 47, 62, 60, 47, 97, 58, 97, 99, 99,
        101, 110, 116, 50, 62, 60, 97, 58, 97, 99,
        99, 101, 110, 116, 51, 62, 60, 97, 58, 115,
        114, 103, 98, 67, 108, 114, 32, 118, 97, 108,
        61, 34, 57, 66, 66, 66, 53, 57, 34, 47,
        62, 60, 47, 97, 58, 97, 99, 99, 101, 110,
        116, 51, 62, 60, 97, 58, 97, 99, 99, 101,
        110, 116, 52, 62, 60, 97, 58, 115, 114, 103,
        98, 67, 108, 114, 32, 118, 97, 108, 61, 34,
        56, 48, 54, 52, 65, 50, 34, 47, 62, 60,
        47, 97, 58, 97, 99, 99, 101, 110, 116, 52,
        62, 60, 97, 58, 97, 99, 99, 101, 110, 116,
        53, 62, 60, 97, 58, 115, 114, 103, 98, 67,
        108, 114, 32, 118, 97, 108, 61, 34, 52, 66,
        65, 67, 67, 54, 34, 47, 62, 60, 47, 97,
        58, 97, 99, 99, 101, 110, 116, 53, 62, 60,
        97, 58, 97, 99, 99, 101, 110, 116, 54, 62,
        60, 97, 58, 115, 114, 103, 98, 67, 108, 114,
        32, 118, 97, 108, 61, 34, 70, 55, 57, 54,
        52, 54, 34, 47, 62, 60, 47, 97, 58, 97,
        99, 99, 101, 110, 116, 54, 62, 60, 97, 58,
        104, 108, 105, 110, 107, 62, 60, 97, 58, 115,
        114, 103, 98, 67, 108, 114, 32, 118, 97, 108,
        61, 34, 48, 48, 48, 48, 70, 70, 34, 47,
        62, 60, 47, 97, 58, 104, 108, 105, 110, 107,
        62, 60, 97, 58, 102, 111, 108, 72, 108, 105,
        110, 107, 62, 60, 97, 58, 115, 114, 103, 98,
        67, 108, 114, 32, 118, 97, 108, 61, 34, 56,
        48, 48, 48, 56, 48, 34, 47, 62, 60, 47,
        97, 58, 102, 111, 108, 72, 108, 105, 110, 107,
        62, 60, 47, 97, 58, 99, 108, 114, 83, 99,
        104, 101, 109, 101, 62, 60, 97, 58, 102, 111,
        110, 116, 83, 99, 104, 101, 109, 101, 32, 110,
        97, 109, 101, 61, 34, 79, 102, 102, 105, 99,
        101, 34, 62, 60, 97, 58, 109, 97, 106, 111,
        114, 70, 111, 110, 116, 62, 60, 97, 58, 108,
        97, 116, 105, 110, 32, 116, 121, 112, 101, 102,
        97, 99, 101, 61, 34, 67, 97, 109, 98, 114,
        105, 97, 34, 47, 62, 60, 97, 58, 101, 97,
        32, 116, 121, 112, 101, 102, 97, 99, 101, 61,
        34, 34, 47, 62, 60, 97, 58, 99, 115, 32,
        116, 121, 112, 101, 102, 97, 99, 101, 61, 34,
        34, 47, 62, 60, 97, 58, 102, 111, 110, 116,
        32, 115, 99, 114, 105, 112, 116, 61, 34, 74,
        112, 97, 110, 34, 32, 116, 121, 112, 101, 102,
        97, 99, 101, 61, 34, 239, 188, 173, 239, 188,
        179, 32, 239, 188, 176, 227, 130, 180, 227, 130,
        183, 227, 131, 131, 227, 130, 175, 34, 47, 62,
        60, 97, 58, 102, 111, 110, 116, 32, 115, 99,
        114, 105, 112, 116, 61, 34, 72, 97, 110, 103,
        34, 32, 116, 121, 112, 101, 102, 97, 99, 101,
        61, 34, 235, 167, 145, 236, 157, 128, 32, 234,
        179, 160, 235, 148, 149, 34, 47, 62, 60, 97,
        58, 102, 111, 110, 116, 32, 115, 99, 114, 105,
        112, 116, 61, 34, 72, 97, 110, 115, 34, 32,
        116, 121, 112, 101, 102, 97, 99, 101, 61, 34,
        229, 174, 139, 228, 189, 147, 34, 47, 62, 60,
        97, 58, 102, 111, 110, 116, 32, 115, 99, 114,
        105, 112, 116, 61, 34, 72, 97, 110, 116, 34,
        32, 116, 121, 112, 101, 102, 97, 99, 101, 61,
        34, 230, 150, 176, 231, 180, 176, 230, 152, 142,
        233, 171, 148, 34, 47, 62, 60, 97, 58, 102,
        111, 110, 116, 32, 115, 99, 114, 105, 112, 116,
        61, 34, 65, 114, 97, 98, 34, 32, 116, 121,
        112, 101, 102, 97, 99, 101, 61, 34, 84, 105,
        109, 101, 115, 32, 78, 101, 119, 32, 82, 111,
        109, 97, 110, 34, 47, 62, 60, 97, 58, 102,
        111, 110, 116, 32, 115, 99, 114, 105, 112, 116,
        61, 34, 72, 101, 98, 114, 34, 32, 116, 121,
        112, 101, 102, 97, 99, 101, 61, 34, 84, 105,
        109, 101, 115, 32, 78, 101, 119, 32, 82, 111,
        109, 97, 110, 34, 47, 62, 60, 97, 58, 102,
        111, 110, 116, 32, 115, 99, 114, 105, 112, 116,
        61, 34, 84, 104, 97, 105, 34, 32, 116, 121,
        112, 101, 102, 97, 99, 101, 61, 34, 84, 97,
        104, 111, 109, 97, 34, 47, 62, 60, 97, 58,
        102, 111, 110, 116, 32, 115, 99, 114, 105, 112,
        116, 61, 34, 69, 116, 104, 105, 34, 32, 116,
        121, 112, 101, 102, 97, 99, 101, 61, 34, 78,
        121, 97, 108, 97, 34, 47, 62, 60, 97, 58,
        102, 111, 110, 116, 32, 115, 99, 114, 105, 112,
        116, 61, 34, 66, 101, 110, 103, 34, 32, 116,
        121, 112, 101, 102, 97, 99, 101, 61, 34, 86,
        114, 105, 110, 100, 97, 34, 47, 62, 60, 97,
        58, 102, 111, 110, 116, 32, 115, 99, 114, 105,
        112, 116, 61, 34, 71, 117, 106, 114, 34, 32,
        116, 121, 112, 101, 102, 97, 99, 101, 61, 34,
        83, 104, 114, 117, 116, 105, 34, 47, 62, 60,
        97, 58, 102, 111, 110, 116, 32, 115, 99, 114,
        105, 112, 116, 61, 34, 75, 104, 109, 114, 34,
        32, 116, 121, 112, 101, 102, 97, 99, 101, 61,
        34, 77, 111, 111, 108, 66, 111, 114, 97, 110,
        34, 47, 62, 60, 97, 58, 102, 111, 110, 116,
        32, 115, 99, 114, 105, 112, 116, 61, 34, 75,
        110, 100, 97, 34, 32, 116, 121, 112, 101, 102,
        97, 99, 101, 61, 34, 84, 117, 110, 103, 97,
        34, 47, 62, 60, 97, 58, 102, 111, 110, 116,
        32, 115, 99, 114, 105, 112, 116, 61, 34, 71,
        117, 114, 117, 34, 32, 116, 121, 112, 101, 102,
        97, 99, 101, 61, 34, 82, 97, 97, 118, 105,
        34, 47, 62, 60, 97, 58, 102, 111, 110, 116,
        32, 115, 99, 114, 105, 112, 116, 61, 34, 67,
        97, 110, 115, 34, 32, 116, 121, 112, 101, 102,
        97, 99, 101, 61, 34, 69, 117, 112, 104, 101,
        109, 105, 97, 34, 47, 62, 60, 97, 58, 102,
        111, 110, 116, 32, 115, 99, 114, 105, 112, 116,
        61, 34, 67, 104, 101, 114, 34, 32, 116, 121,
        112, 101, 102, 97, 99, 101, 61, 34, 80, 108,
        97, 110, 116, 97, 103, 101, 110, 101, 116, 32,
        67, 104, 101, 114, 111, 107, 101, 101, 34, 47,
        62, 60, 97, 58, 102, 111, 110, 116, 32, 115,
        99, 114, 105, 112, 116, 61, 34, 89, 105, 105,
        105, 34, 32, 116, 121, 112, 101, 102, 97, 99,
        101, 61, 34, 77, 105, 99, 114, 111, 115, 111,
        102, 116, 32, 89, 105, 32, 66, 97, 105, 116,
        105, 34, 47, 62, 60, 97, 58, 102, 111, 110,
        116, 32, 115, 99, 114, 105, 112, 116, 61, 34,
        84, 105, 98, 116, 34, 32, 116, 121, 112, 101,
        102, 97, 99, 101, 61, 34, 77, 105, 99, 114,
        111, 115, 111, 102, 116, 32, 72, 105, 109, 97,
        108, 97, 121, 97, 34, 47, 62, 60, 97, 58,
        102, 111, 110, 116, 32, 115, 99, 114, 105, 112,
        116, 61, 34, 84, 104, 97, 97, 34, 32, 116,
        121, 112, 101, 102, 97, 99, 101, 61, 34, 77,
        86, 32, 66, 111, 108, 105, 34, 47, 62, 60,
        97, 58, 102, 111, 110, 116, 32, 115, 99, 114,
        105, 112, 116, 61, 34, 68, 101, 118, 97, 34,
        32, 116, 121, 112, 101, 102, 97, 99, 101, 61,
        34, 77, 97, 110, 103, 97, 108, 34, 47, 62,
        60, 97, 58, 102, 111, 110, 116, 32, 115, 99,
        114, 105, 112, 116, 61, 34, 84, 101, 108, 117,
        34, 32, 116, 121, 112, 101, 102, 97, 99, 101,
        61, 34, 71, 97, 117, 116, 97, 109, 105, 34,
        47, 62, 60, 97, 58, 102, 111, 110, 116, 32,
        115, 99, 114, 105, 112, 116, 61, 34, 84, 97,
        109, 108, 34, 32, 116, 121, 112, 101, 102, 97,
        99, 101, 61, 34, 76, 97, 116, 104, 97, 34,
        47, 62, 60, 97, 58, 102, 111, 110, 116, 32,
        115, 99, 114, 105, 112, 116, 61, 34, 83, 121,
        114, 99, 34, 32, 116, 121, 112, 101, 102, 97,
        99, 101, 61, 34, 69, 115, 116, 114, 97, 110,
        103, 101, 108, 111, 32, 69, 100, 101, 115, 115,
        97, 34, 47, 62, 60, 97, 58, 102, 111, 110,
        116, 32, 115, 99, 114, 105, 112, 116, 61, 34,
        79, 114, 121, 97, 34, 32, 116, 121, 112, 101,
        102, 97, 99, 101, 61, 34, 75, 97, 108, 105,
        110, 103, 97, 34, 47, 62, 60, 97, 58, 102,
        111, 110, 116, 32, 115, 99, 114, 105, 112, 116,
        61, 34, 77, 108, 121, 109, 34, 32, 116, 121,
        112, 101, 102, 97, 99, 101, 61, 34, 75, 97,
        114, 116, 105, 107, 97, 34, 47, 62, 60, 97,
        58, 102, 111, 110, 116, 32, 115, 99, 114, 105,
        112, 116, 61, 34, 76, 97, 111, 111, 34, 32,
        116, 121, 112, 101, 102, 97, 99, 101, 61, 34,
        68, 111, 107, 67, 104, 97, 109, 112, 97, 34,
        47, 62, 60, 97, 58, 102, 111, 110, 116, 32,
        115, 99, 114, 105, 112, 116, 61, 34, 83, 105,
        110, 104, 34, 32, 116, 121, 112, 101, 102, 97,
        99, 101, 61, 34, 73, 115, 107, 111, 111, 108,
        97, 32, 80, 111, 116, 97, 34, 47, 62, 60,
        97, 58, 102, 111, 110, 116, 32, 115, 99, 114,
        105, 112, 116, 61, 34, 77, 111, 110, 103, 34,
        32, 116, 121, 112, 101, 102, 97, 99, 101, 61,
        34, 77, 111, 110, 103, 111, 108, 105, 97, 110,
        32, 66, 97, 105, 116, 105, 34, 47, 62, 60,
        97, 58, 102, 111, 110, 116, 32, 115, 99, 114,
        105, 112, 116, 61, 34, 86, 105, 101, 116, 34,
        32, 116, 121, 112, 101, 102, 97, 99, 101, 61,
        34, 84, 105, 109, 101, 115, 32, 78, 101, 119,
        32, 82, 111, 109, 97, 110, 34, 47, 62, 60,
        97, 58, 102, 111, 110, 116, 32, 115, 99, 114,
        105, 112, 116, 61, 34, 85, 105, 103, 104, 34,
        32, 116, 121, 112, 101, 102, 97, 99, 101, 61,
        34, 77, 105, 99, 114, 111, 115, 111, 102, 116,
        32, 85, 105, 103, 104, 117, 114, 34, 47, 62,
        60, 47, 97, 58, 109, 97, 106, 111, 114, 70,
        111, 110, 116, 62, 60, 97, 58, 109, 105, 110,
        111, 114, 70, 111, 110, 116, 62, 60, 97, 58,
        108, 97, 116, 105, 110, 32, 116, 121, 112, 101,
        102, 97, 99, 101, 61, 34, 67, 97, 108, 105,
        98, 114, 105, 34, 47, 62, 60, 97, 58, 101,
        97, 32, 116, 121, 112, 101, 102, 97, 99, 101,
        61, 34, 34, 47, 62, 60, 97, 58, 99, 115,
        32, 116, 121, 112, 101, 102, 97, 99, 101, 61,
        34, 34, 47, 62, 60, 97, 58, 102, 111, 110,
        116, 32, 115, 99, 114, 105, 112, 116, 61, 34,
        74, 112, 97, 110, 34, 32, 116, 121, 112, 101,
        102, 97, 99, 101, 61, 34, 239, 188, 173, 239,
        188, 179, 32, 239, 188, 176, 227, 130, 180, 227,
        130, 183, 227, 131, 131, 227, 130, 175, 34, 47,
        62, 60, 97, 58, 102, 111, 110, 116, 32, 115,
        99, 114, 105, 112, 116, 61, 34, 72, 97, 110,
        103, 34, 32, 116, 121, 112, 101, 102, 97, 99,
        101, 61, 34, 235, 167, 145, 236, 157, 128, 32,
        234, 179, 160, 235, 148, 149, 34, 47, 62, 60,
        97, 58, 102, 111, 110, 116, 32, 115, 99, 114,
        105, 112, 116, 61, 34, 72, 97, 110, 115, 34,
        32, 116, 121, 112, 101, 102, 97, 99, 101, 61,
        34, 229, 174, 139, 228, 189, 147, 34, 47, 62,
        60, 97, 58, 102, 111, 110, 116, 32, 115, 99,
        114, 105, 112, 116, 61, 34, 72, 97, 110, 116,
        34, 32, 116, 121, 112, 101, 102, 97, 99, 101,
        61, 34, 230, 150, 176, 231, 180, 176, 230, 152,
        142, 233, 171, 148, 34, 47, 62, 60, 97, 58,
        102, 111, 110, 116, 32, 115, 99, 114, 105, 112,
        116, 61, 34, 65, 114, 97, 98, 34, 32, 116,
        121, 112, 101, 102, 97, 99, 101, 61, 34, 65,
        114, 105, 97, 108, 34, 47, 62, 60, 97, 58,
        102, 111, 110, 116, 32, 115, 99, 114, 105, 112,
        116, 61, 34, 72, 101, 98, 114, 34, 32, 116,
        121, 112, 101, 102, 97, 99, 101, 61, 34, 65,
        114, 105, 97, 108, 34, 47, 62, 60, 97, 58,
        102, 111, 110, 116, 32, 115, 99, 114, 105, 112,
        116, 61, 34, 84, 104, 97, 105, 34, 32, 116,
        121, 112, 101, 102, 97, 99, 101, 61, 34, 84,
        97, 104, 111, 109, 97, 34, 47, 62, 60, 97,
        58, 102, 111, 110, 116, 32, 115, 99, 114, 105,
        112, 116, 61, 34, 69, 116, 104, 105, 34, 32,
        116, 121, 112, 101, 102, 97, 99, 101, 61, 34,
        78, 121, 97, 108, 97, 34, 47, 62, 60, 97,
        58, 102, 111, 110, 116, 32, 115, 99, 114, 105,
        112, 116, 61, 34, 66, 101, 110, 103, 34, 32,
        116, 121, 112, 101, 102, 97, 99, 101, 61, 34,
        86, 114, 105, 110, 100, 97, 34, 47, 62, 60,
        97, 58, 102, 111, 110, 116, 32, 115, 99, 114,
        105, 112, 116, 61, 34, 71, 117, 106, 114, 34,
        32, 116, 121, 112, 101, 102, 97, 99, 101, 61,
        34, 83, 104, 114, 117, 116, 105, 34, 47, 62,
        60, 97, 58, 102, 111, 110, 116, 32, 115, 99,
        114, 105, 112, 116, 61, 34, 75, 104, 109, 114,
        34, 32, 116, 121, 112, 101, 102, 97, 99, 101,
        61, 34, 68, 97, 117, 110, 80, 101, 110, 104,
        34, 47, 62, 60, 97, 58, 102, 111, 110, 116,
        32, 115, 99, 114, 105, 112, 116, 61, 34, 75,
        110, 100, 97, 34, 32, 116, 121, 112, 101, 102,
        97, 99, 101, 61, 34, 84, 117, 110, 103, 97,
        34, 47, 62, 60, 97, 58, 102, 111, 110, 116,
        32, 115, 99, 114, 105, 112, 116, 61, 34, 71,
        117, 114, 117, 34, 32, 116, 121, 112, 101, 102,
        97, 99, 101, 61, 34, 82, 97, 97, 118, 105,
        34, 47, 62, 60, 97, 58, 102, 111, 110, 116,
        32, 115, 99, 114, 105, 112, 116, 61, 34, 67,
        97, 110, 115, 34, 32, 116, 121, 112, 101, 102,
        97, 99, 101, 61, 34, 69, 117, 112, 104, 101,
        109, 105, 97, 34, 47, 62, 60, 97, 58, 102,
        111, 110, 116, 32, 115, 99, 114, 105, 112, 116,
        61, 34, 67, 104, 101, 114, 34, 32, 116, 121,
        112, 101, 102, 97, 99, 101, 61, 34, 80, 108,
        97, 110, 116, 97, 103, 101, 110, 101, 116, 32,
        67, 104, 101, 114, 111, 107, 101, 101, 34, 47,
        62, 60, 97, 58, 102, 111, 110, 116, 32, 115,
        99, 114, 105, 112, 116, 61, 34, 89, 105, 105,
        105, 34, 32, 116, 121, 112, 101, 102, 97, 99,
        101, 61, 34, 77, 105, 99, 114, 111, 115, 111,
        102, 116, 32, 89, 105, 32, 66, 97, 105, 116,
        105, 34, 47, 62, 60, 97, 58, 102, 111, 110,
        116, 32, 115, 99, 114, 105, 112, 116, 61, 34,
        84, 105, 98, 116, 34, 32, 116, 121, 112, 101,
        102, 97, 99, 101, 61, 34, 77, 105, 99, 114,
        111, 115, 111, 102, 116, 32, 72, 105, 109, 97,
        108, 97, 121, 97, 34, 47, 62, 60, 97, 58,
        102, 111, 110, 116, 32, 115, 99, 114, 105, 112,
        116, 61, 34, 84, 104, 97, 97, 34, 32, 116,
        121, 112, 101, 102, 97, 99, 101, 61, 34, 77,
        86, 32, 66, 111, 108, 105, 34, 47, 62, 60,
        97, 58, 102, 111, 110, 116, 32, 115, 99, 114,
        105, 112, 116, 61, 34, 68, 101, 118, 97, 34,
        32, 116, 121, 112, 101, 102, 97, 99, 101, 61,
        34, 77, 97, 110, 103, 97, 108, 34, 47, 62,
        60, 97, 58, 102, 111, 110, 116, 32, 115, 99,
        114, 105, 112, 116, 61, 34, 84, 101, 108, 117,
        34, 32, 116, 121, 112, 101, 102, 97, 99, 101,
        61, 34, 71, 97, 117, 116, 97, 109, 105, 34,
        47, 62, 60, 97, 58, 102, 111, 110, 116, 32,
        115, 99, 114, 105, 112, 116, 61, 34, 84, 97,
        109, 108, 34, 32, 116, 121, 112, 101, 102, 97,
        99, 101, 61, 34, 76, 97, 116, 104, 97, 34,
        47, 62, 60, 97, 58, 102, 111, 110, 116, 32,
        115, 99, 114, 105, 112, 116, 61, 34, 83, 121,
        114, 99, 34, 32, 116, 121, 112, 101, 102, 97,
        99, 101, 61, 34, 69, 115, 116, 114, 97, 110,
        103, 101, 108, 111, 32, 69, 100, 101, 115, 115,
        97, 34, 47, 62, 60, 97, 58, 102, 111, 110,
        116, 32, 115, 99, 114, 105, 112, 116, 61, 34,
        79, 114, 121, 97, 34, 32, 116, 121, 112, 101,
        102, 97, 99, 101, 61, 34, 75, 97, 108, 105,
        110, 103, 97, 34, 47, 62, 60, 97, 58, 102,
        111, 110, 116, 32, 115, 99, 114, 105, 112, 116,
        61, 34, 77, 108, 121, 109, 34, 32, 116, 121,
        112, 101, 102, 97, 99, 101, 61, 34, 75, 97,
        114, 116, 105, 107, 97, 34, 47, 62, 60, 97,
        58, 102, 111, 110, 116, 32, 115, 99, 114, 105,
        112, 116, 61, 34, 76, 97, 111, 111, 34, 32,
        116, 121, 112, 101, 102, 97, 99, 101, 61, 34,
        68, 111, 107, 67, 104, 97, 109, 112, 97, 34,
        47, 62, 60, 97, 58, 102, 111, 110, 116, 32,
        115, 99, 114, 105, 112, 116, 61, 34, 83, 105,
        110, 104, 34, 32, 116, 121, 112, 101, 102, 97,
        99, 101, 61, 34, 73, 115, 107, 111, 111, 108,
        97, 32, 80, 111, 116, 97, 34, 47, 62, 60,
        97, 58, 102, 111, 110, 116, 32, 115, 99, 114,
        105, 112, 116, 61, 34, 77, 111, 110, 103, 34,
        32, 116, 121, 112, 101, 102, 97, 99, 101, 61,
        34, 77, 111, 110, 103, 111, 108, 105, 97, 110,
        32, 66, 97, 105, 116, 105, 34, 47, 62, 60,
        97, 58, 102, 111, 110, 116, 32, 115, 99, 114,
        105, 112, 116, 61, 34, 86, 105, 101, 116, 34,
        32, 116, 121, 112, 101, 102, 97, 99, 101, 61,
        34, 65, 114, 105, 97, 108, 34, 47, 62, 60,
        97, 58, 102, 111, 110, 116, 32, 115, 99, 114,
        105, 112, 116, 61, 34, 85, 105, 103, 104, 34,
        32, 116, 121, 112, 101, 102, 97, 99, 101, 61,
        34, 77, 105, 99, 114, 111, 115, 111, 102, 116,
        32, 85, 105, 103, 104, 117, 114, 34, 47, 62,
        60, 47, 97, 58, 109, 105, 110, 111, 114, 70,
        111, 110, 116, 62, 60, 47, 97, 58, 102, 111,
        110, 116, 83, 99, 104, 101, 109, 101, 62, 60,
        97, 58, 102, 109, 116, 83, 99, 104, 101, 109,
        101, 32, 110, 97, 109, 101, 61, 34, 79, 102,
        102, 105, 99, 101, 34, 62, 60, 97, 58, 102,
        105, 108, 108, 83, 116, 121, 108, 101, 76, 115,
        116, 62, 60, 97, 58, 115, 111, 108, 105, 100,
        70, 105, 108, 108, 62, 60, 97, 58, 115, 99,
        104, 101, 109, 101, 67, 108, 114, 32, 118, 97,
        108, 61, 34, 112, 104, 67, 108, 114, 34, 47,
        62, 60, 47, 97, 58, 115, 111, 108, 105, 100,
        70, 105, 108, 108, 62, 60, 97, 58, 103, 114,
        97, 100, 70, 105, 108, 108, 32, 114, 111, 116,
        87, 105, 116, 104, 83, 104, 97, 112, 101, 61,
        34, 49, 34, 62, 60, 97, 58, 103, 115, 76,
        115, 116, 62, 60, 97, 58, 103, 115, 32, 112,
        111, 115, 61, 34, 48, 34, 62, 60, 97, 58,
        115, 99, 104, 101, 109, 101, 67, 108, 114, 32,
        118, 97, 108, 61, 34, 112, 104, 67, 108, 114,
        34, 62, 60, 97, 58, 116, 105, 110, 116, 32,
        118, 97, 108, 61, 34, 53, 48, 48, 48, 48,
        34, 47, 62, 60, 97, 58, 115, 97, 116, 77,
        111, 100, 32, 118, 97, 108, 61, 34, 51, 48,
        48, 48, 48, 48, 34, 47, 62, 60, 47, 97,
        58, 115, 99, 104, 101, 109, 101, 67, 108, 114,
        62, 60, 47, 97, 58, 103, 115, 62, 60, 97,
        58, 103, 115, 32, 112, 111, 115, 61, 34, 51,
        53, 48, 48, 48, 34, 62, 60, 97, 58, 115,
        99, 104, 101, 109, 101, 67, 108, 114, 32, 118,
        97, 108, 61, 34, 112, 104, 67, 108, 114, 34,
        62, 60, 97, 58, 116, 105, 110, 116, 32, 118,
        97, 108, 61, 34, 51, 55, 48, 48, 48, 34,
        47, 62, 60, 97, 58, 115, 97, 116, 77, 111,
        100, 32, 118, 97, 108, 61, 34, 51, 48, 48,
        48, 48, 48, 34, 47, 62, 60, 47, 97, 58,
        115, 99, 104, 101, 109, 101, 67, 108, 114, 62,
        60, 47, 97, 58, 103, 115, 62, 60, 97, 58,
        103, 115, 32, 112, 111, 115, 61, 34, 49, 48,
        48, 48, 48, 48, 34, 62, 60, 97, 58, 115,
        99, 104, 101, 109, 101, 67, 108, 114, 32, 118,
        97, 108, 61, 34, 112, 104, 67, 108, 114, 34,
        62, 60, 97, 58, 116, 105, 110, 116, 32, 118,
        97, 108, 61, 34, 49, 53, 48, 48, 48, 34,
        47, 62, 60, 97, 58, 115, 97, 116, 77, 111,
        100, 32, 118, 97, 108, 61, 34, 51, 53, 48,
        48, 48, 48, 34, 47, 62, 60, 47, 97, 58,
        115, 99, 104, 101, 109, 101, 67, 108, 114, 62,
        60, 47, 97, 58, 103, 115, 62, 60, 47, 97,
        58, 103, 115, 76, 115, 116, 62, 60, 97, 58,
        108, 105, 110, 32, 97, 110, 103, 61, 34, 49,
        54, 50, 48, 48, 48, 48, 48, 34, 32, 115,
        99, 97, 108, 101, 100, 61, 34, 49, 34, 47,
        62, 60, 47, 97, 58, 103, 114, 97, 100, 70,
        105, 108, 108, 62, 60, 97, 58, 103, 114, 97,
        100, 70, 105, 108, 108, 32, 114, 111, 116, 87,
        105, 116, 104, 83, 104, 97, 112, 101, 61, 34,
        49, 34, 62, 60, 97, 58, 103, 115, 76, 115,
        116, 62, 60, 97, 58, 103, 115, 32, 112, 111,
        115, 61, 34, 48, 34, 62, 60, 97, 58, 115,
        99, 104, 101, 109, 101, 67, 108, 114, 32, 118,
        97, 108, 61, 34, 112, 104, 67, 108, 114, 34,
        62, 60, 97, 58, 115, 104, 97, 100, 101, 32,
        118, 97, 108, 61, 34, 53, 49, 48, 48, 48,
        34, 47, 62, 60, 97, 58, 115, 97, 116, 77,
        111, 100, 32, 118, 97, 108, 61, 34, 49, 51,
        48, 48, 48, 48, 34, 47, 62, 60, 47, 97,
        58, 115, 99, 104, 101, 109, 101, 67, 108, 114,
        62, 60, 47, 97, 58, 103, 115, 62, 60, 97,
        58, 103, 115, 32, 112, 111, 115, 61, 34, 56,
        48, 48, 48, 48, 34, 62, 60, 97, 58, 115,
        99, 104, 101, 109, 101, 67, 108, 114, 32, 118,
        97, 108, 61, 34, 112, 104, 67, 108, 114, 34,
        62, 60, 97, 58, 115, 104, 97, 100, 101, 32,
        118, 97, 108, 61, 34, 57, 51, 48, 48, 48,
        34, 47, 62, 60, 97, 58, 115, 97, 116, 77,
        111, 100, 32, 118, 97, 108, 61, 34, 49, 51,
        48, 48, 48, 48, 34, 47, 62, 60, 47, 97,
        58, 115, 99, 104, 101, 109, 101, 67, 108, 114,
        62, 60, 47, 97, 58, 103, 115, 62, 60, 97,
        58, 103, 115, 32, 112, 111, 115, 61, 34, 49,
        48, 48, 48, 48, 48, 34, 62, 60, 97, 58,
        115, 99, 104, 101, 109, 101, 67, 108, 114, 32,
        118, 97, 108, 61, 34, 112, 104, 67, 108, 114,
        34, 62, 60, 97, 58, 115, 104, 97, 100, 101,
        32, 118, 97, 108, 61, 34, 57, 52, 48, 48,
        48, 34, 47, 62, 60, 97, 58, 115, 97, 116,
        77, 111, 100, 32, 118, 97, 108, 61, 34, 49,
        51, 53, 48, 48, 48, 34, 47, 62, 60, 47,
        97, 58, 115, 99, 104, 101, 109, 101, 67, 108,
        114, 62, 60, 47, 97, 58, 103, 115, 62, 60,
        47, 97, 58, 103, 115, 76, 115, 116, 62, 60,
        97, 58, 108, 105, 110, 32, 97, 110, 103, 61,
        34, 49, 54, 50, 48, 48, 48, 48, 48, 34,
        32, 115, 99, 97, 108, 101, 100, 61, 34, 48,
        34, 47, 62, 60, 47, 97, 58, 103, 114, 97,
        100, 70, 105, 108, 108, 62, 60, 47, 97, 58,
        102, 105, 108, 108, 83, 116, 121, 108, 101, 76,
        115, 116, 62, 60, 97, 58, 108, 110, 83, 116,
        121, 108, 101, 76, 115, 116, 62, 60, 97, 58,
        108, 110, 32, 119, 61, 34, 57, 53, 50, 53,
        34, 32, 99, 97, 112, 61, 34, 102, 108, 97,
        116, 34, 32, 99, 109, 112, 100, 61, 34, 115,
        110, 103, 34, 32, 97, 108, 103, 110, 61, 34,
        99, 116, 114, 34, 62, 60, 97, 58, 115, 111,
        108, 105, 100, 70, 105, 108, 108, 62, 60, 97,
        58, 115, 99, 104, 101, 109, 101, 67, 108, 114,
        32, 118, 97, 108, 61, 34, 112, 104, 67, 108,
        114, 34, 62, 60, 97, 58, 115, 104, 97, 100,
        101, 32, 118, 97, 108, 61, 34, 57, 53, 48,
        48, 48, 34, 47, 62, 60, 97, 58, 115, 97,
        116, 77, 111, 100, 32, 118, 97, 108, 61, 34,
        49, 48, 53, 48, 48, 48, 34, 47, 62, 60,
        47, 97, 58, 115, 99, 104, 101, 109, 101, 67,
        108, 114, 62, 60, 47, 97, 58, 115, 111, 108,
        105, 100, 70, 105, 108, 108, 62, 60, 97, 58,
        112, 114, 115, 116, 68, 97, 115, 104, 32, 118,
        97, 108, 61, 34, 115, 111, 108, 105, 100, 34,
        47, 62, 60, 47, 97, 58, 108, 110, 62, 60,
        97, 58, 108, 110, 32, 119, 61, 34, 50, 53,
        52, 48, 48, 34, 32, 99, 97, 112, 61, 34,
        102, 108, 97, 116, 34, 32, 99, 109, 112, 100,
        61, 34, 115, 110, 103, 34, 32, 97, 108, 103,
        110, 61, 34, 99, 116, 114, 34, 62, 60, 97,
        58, 115, 111, 108, 105, 100, 70, 105, 108, 108,
        62, 60, 97, 58, 115, 99, 104, 101, 109, 101,
        67, 108, 114, 32, 118, 97, 108, 61, 34, 112,
        104, 67, 108, 114, 34, 47, 62, 60, 47, 97,
        58, 115, 111, 108, 105, 100, 70, 105, 108, 108,
        62, 60, 97, 58, 112, 114, 115, 116, 68, 97,
        115, 104, 32, 118, 97, 108, 61, 34, 115, 111,
        108, 105, 100, 34, 47, 62, 60, 47, 97, 58,
        108, 110, 62, 60, 97, 58, 108, 110, 32, 119,
        61, 34, 51, 56, 49, 48, 48, 34, 32, 99,
        97, 112, 61, 34, 102, 108, 97, 116, 34, 32,
        99, 109, 112, 100, 61, 34, 115, 110, 103, 34,
        32, 97, 108, 103, 110, 61, 34, 99, 116, 114,
        34, 62, 60, 97, 58, 115, 111, 108, 105, 100,
        70, 105, 108, 108, 62, 60, 97, 58, 115, 99,
        104, 101, 109, 101, 67, 108, 114, 32, 118, 97,
        108, 61, 34, 112, 104, 67, 108, 114, 34, 47,
        62, 60, 47, 97, 58, 115, 111, 108, 105, 100,
        70, 105, 108, 108, 62, 60, 97, 58, 112, 114,
        115, 116, 68, 97, 115, 104, 32, 118, 97, 108,
        61, 34, 115, 111, 108, 105, 100, 34, 47, 62,
        60, 47, 97, 58, 108, 110, 62, 60, 47, 97,
        58, 108, 110, 83, 116, 121, 108, 101, 76, 115,
        116, 62, 60, 97, 58, 101, 102, 102, 101, 99,
        116, 83, 116, 121, 108, 101, 76, 115, 116, 62,
        60, 97, 58, 101, 102, 102, 101, 99, 116, 83,
        116, 121, 108, 101, 62, 60, 97, 58, 101, 102,
        102, 101, 99, 116, 76, 115, 116, 62, 60, 97,
        58, 111, 117, 116, 101, 114, 83, 104, 100, 119,
        32, 98, 108, 117, 114, 82, 97, 100, 61, 34,
        52, 48, 48, 48, 48, 34, 32, 100, 105, 115,
        116, 61, 34, 50, 48, 48, 48, 48, 34, 32,
        100, 105, 114, 61, 34, 53, 52, 48, 48, 48,
        48, 48, 34, 32, 114, 111, 116, 87, 105, 116,
        104, 83, 104, 97, 112, 101, 61, 34, 48, 34,
        62, 60, 97, 58, 115, 114, 103, 98, 67, 108,
        114, 32, 118, 97, 108, 61, 34, 48, 48, 48,
        48, 48, 48, 34, 62, 60, 97, 58, 97, 108,
        112, 104, 97, 32, 118, 97, 108, 61, 34, 51,
        56, 48, 48, 48, 34, 47, 62, 60, 47, 97,
        58, 115, 114, 103, 98, 67, 108, 114, 62, 60,
        47, 97, 58, 111, 117, 116, 101, 114, 83, 104,
        100, 119, 62, 60, 47, 97, 58, 101, 102, 102,
        101, 99, 116, 76, 115, 116, 62, 60, 47, 97,
        58, 101, 102, 102, 101, 99, 116, 83, 116, 121,
        108, 101, 62, 60, 97, 58, 101, 102, 102, 101,
        99, 116, 83, 116, 121, 108, 101, 62, 60, 97,
        58, 101, 102, 102, 101, 99, 116, 76, 115, 116,
        62, 60, 97, 58, 111, 117, 116, 101, 114, 83,
        104, 100, 119, 32, 98, 108, 117, 114, 82, 97,
        100, 61, 34, 52, 48, 48, 48, 48, 34, 32,
        100, 105, 115, 116, 61, 34, 50, 51, 48, 48,
        48, 34, 32, 100, 105, 114, 61, 34, 53, 52,
        48, 48, 48, 48, 48, 34, 32, 114, 111, 116,
        87, 105, 116, 104, 83, 104, 97, 112, 101, 61,
        34, 48, 34, 62, 60, 97, 58, 115, 114, 103,
        98, 67, 108, 114, 32, 118, 97, 108, 61, 34,
        48, 48, 48, 48, 48, 48, 34, 62, 60, 97,
        58, 97, 108, 112, 104, 97, 32, 118, 97, 108,
        61, 34, 51, 53, 48, 48, 48, 34, 47, 62,
        60, 47, 97, 58, 115, 114, 103, 98, 67, 108,
        114, 62, 60, 47, 97, 58, 111, 117, 116, 101,
        114, 83, 104, 100, 119, 62, 60, 47, 97, 58,
        101, 102, 102, 101, 99, 116, 76, 115, 116, 62,
        60, 47, 97, 58, 101, 102, 102, 101, 99, 116,
        83, 116, 121, 108, 101, 62, 60, 97, 58, 101,
        102, 102, 101, 99, 116, 83, 116, 121, 108, 101,
        62, 60, 97, 58, 101, 102, 102, 101, 99, 116,
        76, 115, 116, 62, 60, 97, 58, 111, 117, 116,
        101, 114, 83, 104, 100, 119, 32, 98, 108, 117,
        114, 82, 97, 100, 61, 34, 52, 48, 48, 48,
        48, 34, 32, 100, 105, 115, 116, 61, 34, 50,
        51, 48, 48, 48, 34, 32, 100, 105, 114, 61,
        34, 53, 52, 48, 48, 48, 48, 48, 34, 32,
        114, 111, 116, 87, 105, 116, 104, 83, 104, 97,
        112, 101, 61, 34, 48, 34, 62, 60, 97, 58,
        115, 114, 103, 98, 67, 108, 114, 32, 118, 97,
        108, 61, 34, 48, 48, 48, 48, 48, 48, 34,
        62, 60, 97, 58, 97, 108, 112, 104, 97, 32,
        118, 97, 108, 61, 34, 51, 53, 48, 48, 48,
        34, 47, 62, 60, 47, 97, 58, 115, 114, 103,
        98, 67, 108, 114, 62, 60, 47, 97, 58, 111,
        117, 116, 101, 114, 83, 104, 100, 119, 62, 60,
        47, 97, 58, 101, 102, 102, 101, 99, 116, 76,
        115, 116, 62, 60, 97, 58, 115, 99, 101, 110,
        101, 51, 100, 62, 60, 97, 58, 99, 97, 109,
        101, 114, 97, 32, 112, 114, 115, 116, 61, 34,
        111, 114, 116, 104, 111, 103, 114, 97, 112, 104,
        105, 99, 70, 114, 111, 110, 116, 34, 62, 60,
        97, 58, 114, 111, 116, 32, 108, 97, 116, 61,
        34, 48, 34, 32, 108, 111, 110, 61, 34, 48,
        34, 32, 114, 101, 118, 61, 34, 48, 34, 47,
        62, 60, 47, 97, 58, 99, 97, 109, 101, 114,
        97, 62, 60, 97, 58, 108, 105, 103, 104, 116,
        82, 105, 103, 32, 114, 105, 103, 61, 34, 116,
        104, 114, 101, 101, 80, 116, 34, 32, 100, 105,
        114, 61, 34, 116, 34, 62, 60, 97, 58, 114,
        111, 116, 32, 108, 97, 116, 61, 34, 48, 34,
        32, 108, 111, 110, 61, 34, 48, 34, 32, 114,
        101, 118, 61, 34, 49, 50, 48, 48, 48, 48,
        48, 34, 47, 62, 60, 47, 97, 58, 108, 105,
        103, 104, 116, 82, 105, 103, 62, 60, 47, 97,
        58, 115, 99, 101, 110, 101, 51, 100, 62, 60,
        97, 58, 115, 112, 51, 100, 62, 60, 97, 58,
        98, 101, 118, 101, 108, 84, 32, 119, 61, 34,
        54, 51, 53, 48, 48, 34, 32, 104, 61, 34,
        50, 53, 52, 48, 48, 34, 47, 62, 60, 47,
        97, 58, 115, 112, 51, 100, 62, 60, 47, 97,
        58, 101, 102, 102, 101, 99, 116, 83, 116, 121,
        108, 101, 62, 60, 47, 97, 58, 101, 102, 102,
        101, 99, 116, 83, 116, 121, 108, 101, 76, 115,
        116, 62, 60, 97, 58, 98, 103, 70, 105, 108,
        108, 83, 116, 121, 108, 101, 76, 115, 116, 62,
        60, 97, 58, 115, 111, 108, 105, 100, 70, 105,
        108, 108, 62, 60, 97, 58, 115, 99, 104, 101,
        109, 101, 67, 108, 114, 32, 118, 97, 108, 61,
        34, 112, 104, 67, 108, 114, 34, 47, 62, 60,
        47, 97, 58, 115, 111, 108, 105, 100, 70, 105,
        108, 108, 62, 60, 97, 58, 103, 114, 97, 100,
        70, 105, 108, 108, 32, 114, 111, 116, 87, 105,
        116, 104, 83, 104, 97, 112, 101, 61, 34, 49,
        34, 62, 60, 97, 58, 103, 115, 76, 115, 116,
        62, 60, 97, 58, 103, 115, 32, 112, 111, 115,
        61, 34, 48, 34, 62, 60, 97, 58, 115, 99,
        104, 101, 109, 101, 67, 108, 114, 32, 118, 97,
        108, 61, 34, 112, 104, 67, 108, 114, 34, 62,
        60, 97, 58, 116, 105, 110, 116, 32, 118, 97,
        108, 61, 34, 52, 48, 48, 48, 48, 34, 47,
        62, 60, 97, 58, 115, 97, 116, 77, 111, 100,
        32, 118, 97, 108, 61, 34, 51, 53, 48, 48,
        48, 48, 34, 47, 62, 60, 47, 97, 58, 115,
        99, 104, 101, 109, 101, 67, 108, 114, 62, 60,
        47, 97, 58, 103, 115, 62, 60, 97, 58, 103,
        115, 32, 112, 111, 115, 61, 34, 52, 48, 48,
        48, 48, 34, 62, 60, 97, 58, 115, 99, 104,
        101, 109, 101, 67, 108, 114, 32, 118, 97, 108,
        61, 34, 112, 104, 67, 108, 114, 34, 62, 60,
        97, 58, 116, 105, 110, 116, 32, 118, 97, 108,
        61, 34, 52, 53, 48, 48, 48, 34, 47, 62,
        60, 97, 58, 115, 104, 97, 100, 101, 32, 118,
        97, 108, 61, 34, 57, 57, 48, 48, 48, 34,
        47, 62, 60, 97, 58, 115, 97, 116, 77, 111,
        100, 32, 118, 97, 108, 61, 34, 51, 53, 48,
        48, 48, 48, 34, 47, 62, 60, 47, 97, 58,
        115, 99, 104, 101, 109, 101, 67, 108, 114, 62,
        60, 47, 97, 58, 103, 115, 62, 60, 97, 58,
        103, 115, 32, 112, 111, 115, 61, 34, 49, 48,
        48, 48, 48, 48, 34, 62, 60, 97, 58, 115,
        99, 104, 101, 109, 101, 67, 108, 114, 32, 118,
        97, 108, 61, 34, 112, 104, 67, 108, 114, 34,
        62, 60, 97, 58, 115, 104, 97, 100, 101, 32,
        118, 97, 108, 61, 34, 50, 48, 48, 48, 48,
        34, 47, 62, 60, 97, 58, 115, 97, 116, 77,
        111, 100, 32, 118, 97, 108, 61, 34, 50, 53,
        53, 48, 48, 48, 34, 47, 62, 60, 47, 97,
        58, 115, 99, 104, 101, 109, 101, 67, 108, 114,
        62, 60, 47, 97, 58, 103, 115, 62, 60, 47,
        97, 58, 103, 115, 76, 115, 116, 62, 60, 97,
        58, 112, 97, 116, 104, 32, 112, 97, 116, 104,
        61, 34, 99, 105, 114, 99, 108, 101, 34, 62,
        60, 97, 58, 102, 105, 108, 108, 84, 111, 82,
        101, 99, 116, 32, 108, 61, 34, 53, 48, 48,
        48, 48, 34, 32, 116, 61, 34, 45, 56, 48,
        48, 48, 48, 34, 32, 114, 61, 34, 53, 48,
        48, 48, 48, 34, 32, 98, 61, 34, 49, 56,
        48, 48, 48, 48, 34, 47, 62, 60, 47, 97,
        58, 112, 97, 116, 104, 62, 60, 47, 97, 58,
        103, 114, 97, 100, 70, 105, 108, 108, 62, 60,
        97, 58, 103, 114, 97, 100, 70, 105, 108, 108,
        32, 114, 111, 116, 87, 105, 116, 104, 83, 104,
        97, 112, 101, 61, 34, 49, 34, 62, 60, 97,
        58, 103, 115, 76, 115, 116, 62, 60, 97, 58,
        103, 115, 32, 112, 111, 115, 61, 34, 48, 34,
        62, 60, 97, 58, 115, 99, 104, 101, 109, 101,
        67, 108, 114, 32, 118, 97, 108, 61, 34, 112,
        104, 67, 108, 114, 34, 62, 60, 97, 58, 116,
        105, 110, 116, 32, 118, 97, 108, 61, 34, 56,
        48, 48, 48, 48, 34, 47, 62, 60, 97, 58,
        115, 97, 116, 77, 111, 100, 32, 118, 97, 108,
        61, 34, 51, 48, 48, 48, 48, 48, 34, 47,
        62, 60, 47, 97, 58, 115, 99, 104, 101, 109,
        101, 67, 108, 114, 62, 60, 47, 97, 58, 103,
        115, 62, 60, 97, 58, 103, 115, 32, 112, 111,
        115, 61, 34, 49, 48, 48, 48, 48, 48, 34,
        62, 60, 97, 58, 115, 99, 104, 101, 109, 101,
        67, 108, 114, 32, 118, 97, 108, 61, 34, 112,
        104, 67, 108, 114, 34, 62, 60, 97, 58, 115,
        104, 97, 100, 101, 32, 118, 97, 108, 61, 34,
        51, 48, 48, 48, 48, 34, 47, 62, 60, 97,
        58, 115, 97, 116, 77, 111, 100, 32, 118, 97,
        108, 61, 34, 50, 48, 48, 48, 48, 48, 34,
        47, 62, 60, 47, 97, 58, 115, 99, 104, 101,
        109, 101, 67, 108, 114, 62, 60, 47, 97, 58,
        103, 115, 62, 60, 47, 97, 58, 103, 115, 76,
        115, 116, 62, 60, 97, 58, 112, 97, 116, 104,
        32, 112, 97, 116, 104, 61, 34, 99, 105, 114,
        99, 108, 101, 34, 62, 60, 97, 58, 102, 105,
        108, 108, 84, 111, 82, 101, 99, 116, 32, 108,
        61, 34, 53, 48, 48, 48, 48, 34, 32, 116,
        61, 34, 53, 48, 48, 48, 48, 34, 32, 114,
        61, 34, 53, 48, 48, 48, 48, 34, 32, 98,
        61, 34, 53, 48, 48, 48, 48, 34, 47, 62,
        60, 47, 97, 58, 112, 97, 116, 104, 62, 60,
        47, 97, 58, 103, 114, 97, 100, 70, 105, 108,
        108, 62, 60, 47, 97, 58, 98, 103, 70, 105,
        108, 108, 83, 116, 121, 108, 101, 76, 115, 116,
        62, 60, 47, 97, 58, 102, 109, 116, 83, 99,
        104, 101, 109, 101, 62, 60, 47, 97, 58, 116,
        104, 101, 109, 101, 69, 108, 101, 109, 101, 110,
        116, 115, 62, 60, 97, 58, 111, 98, 106, 101,
        99, 116, 68, 101, 102, 97, 117, 108, 116, 115,
        47, 62, 60, 97, 58, 101, 120, 116, 114, 97,
        67, 108, 114, 83, 99, 104, 101, 109, 101, 76,
        115, 116, 47, 62, 60, 47, 97, 58, 116, 104,
        101, 109, 101, 62
    }};
    
    return std::string(data.begin(), data.end());;
}

}
