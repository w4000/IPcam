$(document).ready(function () {
});

function isvalid_ip(ipvalue) {
  var ippattern = /^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$/;
  var iparray = ipvalue.match(ippattern);

  var result = "";
  if (ipvalue == "0.0.0.0") return false;
  else if (ipvalue == "255.255.255.255") return false;

  if (iparray == null) {
    return false;
  } else {
    for (i = 0; i < 4; i++) {
      thissegment = iparray[i];
      if (thissegment > 255) return false;
      else if (i == 0 && thissegment > 255) return false;
    }
  }
  return true;
}


function input_korean_check(data, len) {
  var tmp;

  if (len == 0) {
    return 0;
  }

  for (i = 0; i < len; i++) {
    var c = data.charCodeAt(i);
    tmp = 0;

    if (!((0xac00 <= c && c <= 0xd7a3) || (0x3131 <= c && c <= 0x318e))) {
    } else {
      return 1;
    }
  }

  return 0;
}

function idrule_check(usr_id) {
  var len = 0;
  var i, j;
  var nr_alpha = 0,
    nr_num = 0,
    nr_spc = 0,
    nr_space = 0;
  var spc_char = new Array(
    "126",
    "96",
    "33",
    "64",
    "35",
    "36",
    "37",
    "94",
    "38",
    "40",
    "41",
    "95",
    "43", 
    "123",
    "125",
    "58",
    "34",
    "63", 
    "45",
    "61",
    "91",
    "93",
    "59",
    "39",
    "44",
    "46",
    "47"
  ); 

  var space_char = new Array("32"); // space
  let usr_char = 0;

  len = usr_id.length;

  if (len <= 0)
    // User name is zero length.
    return -1;

  if (len < 5)
    // Too short user name.
    return -1;

  for (i = 0; i < len; i++) {
    usr_char = usr_id.charCodeAt(i);
    // upper eng, lower eng check
    if (
      (usr_char >= 65 && usr_char <= 90) ||
      (usr_char >= 91 && usr_char <= 122)
    )
      nr_alpha++;

    // num check
    if (usr_char >= 48 && usr_char <= 57) nr_num++;

    // spcial charcter check
    for (j = 0; j < spc_char.length; j++) {
      if (usr_char == spc_char[j]) nr_spc++;
    }

    // space charcter check
    for (j = 0; j < space_char.length; j++) {
      if (usr_char == space_char[j]) nr_space++;
    }
  }

  if (nr_alpha < 1 || nr_num < 1 || nr_space > 0 || nr_spc > 0) return -1;

  return 1;
}

function is_repeat_character(usr_pwd) {

  let MAX_REPEAT = 3;
  let pwd_len = usr_pwd.length;
  let fchar = 0;
  let i;
  let rpt_cnt = 0;

  for (i = 0; i < pwd_len; i++) {
    if (rpt_cnt >= MAX_REPEAT) return -1;

    

    if (fchar == usr_pwd.charCodeAt(i)) rpt_cnt++;
    else rpt_cnt = 1;
    fchar = usr_pwd.charCodeAt(i);

  }

  return 0;
}

function pwdrule_check(usr_pwd) {
  // repeat
  if (is_repeat_character(usr_pwd) < 0) {
    return -1;
  }


  return 1;
}

function pwd_insert_rule(usr_pwd) {
  let spc_char = new Array(
    "61",
    "91",
    "93",
    "59",
    "39",
    "47",
    "96", 
    "43",
    "123",
    "125",
    "92",
    "126",
    "124",
    "58",
    "34",
    "63",
    "<",
    ">"
  ); 

  pwd_len = usr_pwd.length;

  for (i = 0; i < pwd_len; i++) {
    for (j = 0; j < spc_char.length; j++) {
      if (usr_pwd.charCodeAt(i) == spc_char[j]) return 1;
    }
  }

  return 0;
}
