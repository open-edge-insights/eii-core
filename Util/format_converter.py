
"""
Copyright (c) 2018 Intel Corporation.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
"""

import re


def lf_to_json_converter(data):

        '''Converts line protocol data to json format
        Argument:
            data: line protocol data.
        '''

        final_data = "Measurement="
        jbuf = data.split(" ")
        # To handle json if tags are given in the line protocol
        matchString = re.match(r'([a-zA-Z0-9_]+)([,])([a-zA-Z0-9_]*)', jbuf[0])
        if matchString:
            firstGroup = matchString.group().split(",")
            quotedFirstGroup = "\"" + firstGroup[0] + "\""
            jbuf[0] = jbuf[0].replace(firstGroup[0], quotedFirstGroup)
            tag_value = jbuf[0].split("=")
            quotedt_tag_value = "\"" + tag_value[1] + "\""

            tag_value[1] = tag_value[1].replace(
                tag_value[1], quotedt_tag_value)

            jbuf[0] = tag_value[0] + "=" + tag_value[1]
            final_data += jbuf[0] + ","
        else:
            final_data += "\"" + jbuf[0] + "\"" + ","

        for i in range(2, len(jbuf)):
            jbuf[1] += " " + jbuf[i]

        influxTS = ",influx_ts=" + jbuf[len(jbuf)-1]
        jbuf[1] = jbuf[1].replace(jbuf[len(jbuf)-1], influxTS)
        final_data = final_data + jbuf[1]

        key_value_buf = final_data.split("=")
        quoted_key = "\"" + key_value_buf[0] + "\""
        final_data = final_data.replace(key_value_buf[0], quoted_key)

        for j in range(1, len(key_value_buf)-1):
            key_buf = key_value_buf[j].split(",")
            quoted_key2 = "\"" + key_buf[len(key_buf)-1] + "\""
            final_data = final_data.replace(key_buf[
                len(key_buf)-1], quoted_key2)

        final_data = final_data.replace("=", ":")
        variable = re.findall(r'[0-9]+i', final_data)
        for i in variable:
                stripped_i = i.strip("i")
                final_data = final_data.replace(i, stripped_i)
        final_data = "{" + final_data + "}"

        return final_data
