using System;
using System.Collections.Generic;
using System.Text;
using System.Text.RegularExpressions;
using System.IO;
using System.Linq;

namespace fix_biliobs_zh_ts
{
    class Program
    {
        static void Main(string[] args)
        {
            string[] classNames = File.ReadAllLines("QTClasses.txt");

            string[] lines = File.ReadAllLines("biliobs_zh.ts");
            string currentClassName = null;
            string contextEnd = "</context>";
            Regex nameTag = new Regex(@"[\t ]*<name>([^<]+)</name>");

            int lineCount = lines.Length;
            for (int i = 0; i < lineCount; ++i)
            {
                Match m = nameTag.Match(lines[i]);
                if (m.Success)
                    currentClassName = m.Groups[1].Value;
                else
                    if (lines[i].Equals(contextEnd))
                        currentClassName = null;
                    else
                    {
                        if (string.IsNullOrEmpty(currentClassName) == false
                            && classNames.Contains(currentClassName)
                            )
                        {
                            lines[i] = lines[i].Replace("<translation type=\"vanished\">", "<translation>");
                        }
                    }

            }

            File.Copy("biliobs_zh.ts", "biliobs_zh.ts.bak", true);
            File.WriteAllLines("biliobs_zh.ts", lines);
        }
    }
}
