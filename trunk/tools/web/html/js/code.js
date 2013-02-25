var alphabet ="0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ.,!?() '_-=+*/@$:%^#;~{}[]|"
function toCode(text)
{
	var tlen = text.length,z,ntext="",alen = alphabet.length;
	var sma=getCookie("Msma37");
	if (sma == undefined || isNaN(parseInt(sma))) sma="92711";
	var sid=localStorage.getItem("Msid37");
	if(sid == null || isNaN(parseInt(sid))) sid="95623";
	var kript=hex_md5(sid+sma)
	for(var i = 0; i < tlen; i++)
	{
		z=alphabet.indexOf(text[i]);
		if(z > -1){
			z=z-parseInt(kript[i%kript.length]+kript[(i+1)%kript.length],16);
			z=z%alen;
			if(z<0)z=z+alen;
			ntext = ntext + alphabet[z];
		}
		else{
			ntext=ntext+text[i];
		}
	}
	return ntext;
}
function fromCode(text)
{
	var tlen = text.length,z,ntext="",alen = alphabet.length;
	var sma=getCookie("Msma37");
	if (sma == undefined || isNaN(parseInt(sma))) sma="92711";
	var sid=localStorage.getItem("Msid37");
	if(sid == null || isNaN(parseInt(sid))) sid="95623";
	var kript=hex_md5(sid+sma)
	for(var i = 0; i < tlen; i++)
	{
		z=alphabet.indexOf(text[i]);
		if(z > -1){
			z=z+parseInt(kript[i%kript.length]+kript[(i+1)%kript.length],16);
			z=z%alen;
			ntext = ntext + alphabet[z];
		}
		else{
			ntext=ntext+text[i];
		}
	}
	return ntext;
}