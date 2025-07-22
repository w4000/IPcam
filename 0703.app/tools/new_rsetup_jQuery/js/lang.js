var data_result;

function load_lang_json(lang, page)
{
	var req_url = "../lang/" + lang + ".json";
	$.ajax({
		url: req_url,
		type: "GET",
		dataType: "json",
		cache: false,
		async: false,
		success: function(data) {
			var items = document.querySelectorAll("web-lang");
			for(var i = 0; i < items.length; i++) {
				if(data[page][items[i].textContent]) {
					items[i].textContent = data[page][items[i].textContent];
				}
				else {
					if(data["common"][items[i].textContent]) {
						items[i].textContent = data["common"][items[i].textContent];
					}
				}
				data_result = data[page];
			}
		},
		error:function(err) {
		}
	})
}

function load_language(page)
{
	var req_url = "/_fcgiapi/system/information";
	var lang = "eng";

	$.ajax({
		url: req_url,
		type: "GET",
		dataType: "json",
		cache: false,
		async: false,
		success:function(data) {
			lang = data["system info"].lan;
			load_lang_json(lang, page);
		},
		error:function(err) {
		}
	})
}
