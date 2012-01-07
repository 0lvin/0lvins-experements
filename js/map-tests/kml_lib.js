/* parser for kml(xml) files */
function coordinates_to_points(xmlNode) {
    var points = new Array();

    var fullText = "";

    for(var chunk=0;  chunk < xmlNode.childNodes.length; chunk++)
        fullText += xmlNode.childNodes[chunk].nodeValue;

    var coords = fullText.split(" ");
    for(var j=0; j < coords.length;j++){
        if (coords[j] == '')
            continue;
        var sub_point = coords[j].split(",");
        if ((sub_point[1] != undefined) && (sub_point[0] != undefined)) {
            if (!isNaN(parseFloat(sub_point[1])) && !isNaN(parseFloat(sub_point[0])))
                points.push(
                    {
                        'lat' : parseFloat(sub_point[1]),
                        'lng' : parseFloat(sub_point[0])
                        //'alt' : sub_point[2]
                    }
                );
        }
    }
    return points;
}

function kml_parse_internal(gxml) {
    var parse_result = {};

    //style
    var stylemaps = gxml.documentElement.getElementsByTagName("StyleMap");
    var styles = gxml.documentElement.getElementsByTagName("Style");
    var style_cache = {};
    for(var k=0; k< stylemaps.length; k++)
    {
        var id_style = stylemaps[k].getAttribute('id');
        if (!style_cache[id_style]) {
            var style = stylemaps[k].getElementsByTagName("styleUrl")[0].childNodes[0].nodeValue;
            for(var j = 0; j < styles.length; j++)
            {
                if(styles[j].getAttribute('id') == style.substring(1))
                {
                    var color = styles[j].getElementsByTagName('PolyStyle')[0].getElementsByTagName('color')[0].childNodes[0].nodeValue;
                    var opacity = (parseInt(color.substr(0,2), 16) / 255);
                    color = '#' + color.substr(6,2) + color.substr(4,2) + color.substr(2,2);
                    style_cache[id_style] = {};
                    style_cache[id_style]['color'] = color;
                    style_cache[id_style]['opacity'] = opacity;
                }
            }
        }
    }
    //placemarks
    var placemarks = gxml.documentElement.getElementsByTagName("Placemark");
    for(var i=0; i<placemarks.length; i++){
        var can_add = false;
        var color = "#FF0000";
        var opacity = 0.1;
        var sub_parse_result  = {};

        var style = '';
        if (placemarks[i].getElementsByTagName("styleUrl").length)
            style = placemarks[i].getElementsByTagName("styleUrl")[0].childNodes[0].nodeValue;


        if (placemarks[i].getElementsByTagName("opacity").length) {
            if(jQuery.browser.msie)
            {
                opacity = parseFloat(placemarks[i].getElementsByTagName("opacity")[0].text) ;
            }
            else
            {
                opacity = parseFloat(placemarks[i].getElementsByTagName("opacity")[0].textContent) ;
            }
        }

        if (style_cache[style.substring(1)]) {
            color = style_cache[style.substring(1)]['color'];
            opacity = opacity * style_cache[style.substring(1)]['opacity'];
        }

        sub_parse_result ['color'] = color;
        sub_parse_result ['opacity'] = opacity;

        var index = 0;

        var polygons = placemarks[i].getElementsByTagName("Polygon");
        for(var n = 0; n < polygons.length; n ++ ) {
            var coordinates = polygons[n].getElementsByTagName("coordinates");
            for(var m = 0; m < coordinates.length; m++) {
                var points = coordinates_to_points(coordinates[m]);
                if (points.length) {
                    sub_parse_result [index] = {};
                    sub_parse_result [index]['type'] = 'poligon'
                    sub_parse_result [index]['points'] = points;
                    index ++;
                    can_add = true;
                }
            }
        }

        var linearrings = placemarks[i].getElementsByTagName("LinearRing");
        for(var n = 0; n < linearrings.length; n ++ ) {
            var coordinates = linearrings[n].getElementsByTagName("coordinates");
            for(var m = 0; m < coordinates.length; m++) {
                var points = coordinates_to_points(coordinates[m]);
                if (points.length) {
                    sub_parse_result [index] = {};
                    sub_parse_result [index]['type'] = 'linearring';
                    sub_parse_result [index]['points'] = points;
                    index ++;
                    can_add = true;
                }
            }
        }

        var linestrings = placemarks[i].getElementsByTagName("LineString");
        for(var n = 0; n < linestrings.length; n ++ ) {
            var coordinates = linestrings[n].getElementsByTagName("coordinates");
            for(var m = 0; m < coordinates.length; m++) {
                var points = coordinates_to_points(coordinates[m]);
                if (points.length) {
                    sub_parse_result [index] = {};
                    sub_parse_result [index]['type'] = 'linestring';
                    sub_parse_result [index]['points'] = points;
                    index ++;
                    can_add = true;
                }
            }
        }

        if (!can_add)
            continue;

        if(jQuery.browser.msie)
        {
            var name = placemarks[i].getElementsByTagName("name")[0].text;
        }
        else
        {
            var name = placemarks[i].getElementsByTagName("name")[0].textContent;
        }

        if (!name)
            name = 'unknow_' + i;

        //already exist
        if (parse_result[name])
            name += i;
        parse_result[name] = sub_parse_result;

    }
    return parse_result;
}


