<?php
   function html_compress_without_css($html, $deletenotneededtags = false) {
        $replace_code = '#html'.time().'#';
        preg_match_all('!(<(?:code|pre|textarea).*>[^<]+</(?:code|pre|textarea)>)!',$html,$pre);#exclude pre or code tags
        $html = preg_replace('!<(?:code|pre|textarea).*>[^<]+</(?:code|pre|textarea)>!', $replace_code, $html);#removing all pre or code tags
        $html = preg_replace('#<!--[^\[].+-->#imsU','', $html);#removing HTML comments
        $html = preg_replace('/[\r\n\t]+/', ' ', $html);#remove new lines, spaces, tabs
        $html = preg_replace('/>[\s]+</', '><', $html);#remove new lines, spaces, tabs
	$html = preg_replace('/[\s]+>/', '>', $html);#remove new lines, spaces, tabs
	$html = preg_replace('#[\s]+/>#', '/>', $html);#remove new lines, spaces, tabs
        $html = preg_replace('/[\s]+/', ' ', $html);#remove new lines, spaces, tabs
	$html = preg_replace('/[\s]+&nbsp;[\s]+/', '&nbsp;', $html);
	if ($deletenotneededtags)
	    $html = str_replace(
		array(
		    '</li>', //not needed tag for render document
		    '</p><p', //not needed tag for render document
		    '</body>', //not needed tag for render document
		    '</head>', //not needed tag for render document
		    '</option>' //not needed tag for render document
		),
		array(
		    '',
		    '<p',
		    '',
		    '',
		    ''
		),
		$html
	    );
        if(!empty($pre[0])) {
	    $html = str_replace(' '.$replace_code, $replace_code, $html);
	    $html = str_replace($replace_code.' ', $replace_code, $html);
            foreach($pre[0] as $tag) {
		$tag = str_replace(array('\\', '$'), array('\\\\', '\$'), $tag);
                $html = preg_replace('!'.$replace_code.'!', $tag, $html,1);#putting back pre|code tags
	    }
	}
        return $html;
   }

   function html_compress_without_js($html, $compress_css, $deletenotneededtags = false) {
        $replace_code = '#style'.time().'#';
        preg_match_all('!(<(?:style).*>.*</(?:style)>)!imsU',$html,$pre);#exclude style tags
        $html = preg_replace('!(<(?:style).*>.*</(?:style)>)!imsU', $replace_code, $html);#removing style tags
	$html = html_compress_without_css($html, $deletenotneededtags);
        if(!empty($pre[0])) {
	    $html = str_replace(' '.$replace_code, $replace_code, $html);
	    $html = str_replace($replace_code.' ', $replace_code, $html);
            foreach($pre[0] as $tag) {
		if ($compress_css) {
		    preg_match_all('!<(?:style).*>(.*)</(?:style)>!imsU',$tag,$change);
		    if(count($change) == 2 && count($change[1]) == 1 && !empty($change[1][0])) {
			$packer = CssMin::minify($change[1][0]);
			$packed = str_replace($change[1][0], $packer, $tag);
		    } else {
			$packed = $tag;
		    }
		} else {
		    $packed = $tag;
		}
		$packed = str_replace(array('\\', '$'), array('\\\\', '\$'), $packed);
                $html = preg_replace('!'.$replace_code.'!', $packed, $html,1);#putting back style tags
	    }
	}
        return $html;
   }

   //minimize html except code|pre|script|textarea|style
   function html_compress($html, $compress_js = false, $compress_css = false, $deletenotneededtags = false){
        $replace_code = '#js'.time().'#';
        preg_match_all('!(<(?:script).*>.*</(?:script)>)!imsU',$html,$pre);#exclude script tags
        $html = preg_replace('!(<(?:script).*>.*</(?:script)>)!imsU', $replace_code, $html);#removing script tags
	$html = html_compress_without_js($html, $compress_css, $deletenotneededtags);
        if(!empty($pre[0])) {
	    $html = str_replace(' '.$replace_code, $replace_code, $html);
	    $html = str_replace($replace_code.' ', $replace_code, $html);
            foreach($pre[0] as $tag) {
		if ($compress_js) {
		    preg_match_all('!<(?:script).*>(.*)</(?:script)>!imsU',$tag,$change);
		    if(count($change) == 2 && count($change[1]) == 1 && !empty($change[1][0])) {
			$packed = trim(JSMin::minify($change[1][0]));
			if (strlen($packed) < strlen($change[1][0]))
			    $packed = str_replace($change[1][0], $packed, $tag);
			else
			    $packed = $tag;
		    } else {
			$packed = $tag;
		    }
		} else {
		    $packed = $tag;
		}
		$packed = str_replace(array('\\', '$'), array('\\\\', '\$'), $packed);
                $html = preg_replace('!'.$replace_code.'!', $packed, $html,1);#putting back script tags
	    }
	}
        return $html;
   }
?>
