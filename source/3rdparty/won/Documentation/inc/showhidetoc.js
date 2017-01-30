<SCRIPT>
//<!--
function ToggleList()
{
	if (IE4Verify() == true)
	{
		var child=document.all[event.srcElement.getAttribute("child",false)];
		child.className = ((child.className == "hideTOCLI") ? "showTOCLI" : "hideTOCLI");
	 	var image = event.srcElement.src.lastIndexOf("/images/plus.gif");
		event.srcElement.src = (image >= 0) ? "/images/minus.gif" : "/images/plus.gif";
	}
}
function IE4Verify()
{
	if (navigator.appName.indexOf("Microsoft Internet Explorer") != -1)
	{
		if (navigator.appVersion.indexOf("4.") != -1)
		{
			return true;
	 	}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}


//-->
</SCRIPT>