var alphabet = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.,!?() '_-=+*/@$:%^#;~{}[]|`";
var lalphabet = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ@_-.,`";

function toCode(text){
	var tlen = text.length, z, ntext = "", alen = lalphabet.length;
	var sma = getCookie("Msma37");
	if (sma == undefined) sma = "92711";
	var sid = localStorage.getItem("Msid37");
	if(sid == null) sid = "95623";
	var kript = hex_md5(sid + sma);
	for(; tlen % 32 != 0; tlen++){
		poz = (Math.random() * 100) %(tlen + 1);
		text = text.slice(0, poz) + "`" + text.slice(poz)
	}
	for(var i = 0; i < tlen; i++){
		z = lalphabet.indexOf(text[i]);
		if(z > -1){
			z = z - parseInt(kript[i % kript.length] + kript[(i + 1) % kript.length], 16);
			z = z % alen;
			if(z < 0) z = z + alen;
			ntext = ntext + lalphabet[z];
		}
		else{
			ntext = ntext + text[i];
		}
	}
	return ntext;
}

function fromCode(text, cod){
	if(cod == undefined) code = "";
	var tlen = text.length, z, ntext = "", alen = alphabet.length;
	var sma = getCookie("Msma37");
	if (sma == undefined) sma = "92711";
	var sid = localStorage.getItem("Msid37");
	if(sid == null) sid = "95623";
	var kript = hex_md5(sid + sma + cod);
	for(var i = 0; i < tlen; i++){
		z=alphabet.indexOf(text[i]);
		if(z > -1){
			z = z + parseInt(kript[i % kript.length] + kript[(i + 1) % kript.length],16);
			z = z % alen;
			if(alphabet[z] != "`") ntext = ntext + alphabet[z];
		}
		else{
			ntext = ntext + text[i];
		}
	}
	return ntext;
}
