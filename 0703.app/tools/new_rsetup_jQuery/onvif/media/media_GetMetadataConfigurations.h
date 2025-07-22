
#ifndef MEDIA_GETMETADATACONFIGURATIONS_H_
#define MEDIA_GETMETADATACONFIGURATIONS_H_

int media_GetMetadataConfigurations(FCGX_Stream *out, mxml_node_t *n_cmd)
{
	const char* header =
			"%s"
			"  <SOAP-ENV:Body>\r\n"
			"    <trt:GetMetadataConfigurationsResponse>\r\n";

    const char* body =
            "      <trt:Configurations token=\"%s\">\r\n"
            "        <tt:Name>metaData</tt:Name>\r\n"
            "        <tt:UseCount>1</tt:UseCount>\r\n"
            "        <tt:PTZStatus>\r\n"
            "          <tt:Status>false</tt:Status>\r\n"
            "          <tt:Position>false</tt:Position>\r\n"
            "        </tt:PTZStatus>\r\n"
            "        <tt:Analytics>false</tt:Analytics>\r\n"
            "        <tt:Multicast>\r\n"
            "          <tt:Address>\r\n"
            "            <tt:Type>IPv4</tt:Type>\r\n"
            "            <tt:IPv4Address>0.0.0.0</tt:IPv4Address>\r\n"
            "          </tt:Address>\r\n"
            "          <tt:Port>8864</tt:Port>\r\n"
            "          <tt:TTL>128</tt:TTL>\r\n"
            "          <tt:AutoStart>false</tt:AutoStart>\r\n"
            "        </tt:Multicast>\r\n"
            "        <tt:SessionTimeout>PT5S</tt:SessionTimeout>\r\n"
            "        <tt:AnalyticsEngineConfiguration/>\r\n"
            "      </trt:Configurations>\r\n"
            "      <trt:ForcePersistence>true</trt:ForcePersistence>\r\n";

	const char* footer =
			"    </trt:GetMetadataConfigurationsResponse>\r\n"
			"  </SOAP-ENV:Body>\r\n"
			"</SOAP-ENV:Envelope>\r\n";

	INIT_OUTBUF();

	SET_OUTBUF(header, get_xmlns());
    SET_OUTBUF(body, "MetadataToken");
	SET_OUTBUF(footer);

	Onvif_print_outbuf(out, g_outbuf);

	return 0;
}

#endif /* MEDIA_GETMETADATACONFIGURATIONS_H_ */
