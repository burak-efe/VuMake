using System;
using System.Linq;
using System.IO;

try
{
    Program.SearchStrings();
}
catch (Exception e)
{
    Console.WriteLine(e);
    throw;
}

public static class Program
{
    static string[] paths = new string[]
    {
        "../../src/"
    };

    static string[] searchStrings = new string[]
    {
        "std::vector",
        "std::set",
        "std::stack",
        "std::map",
        "std::unordered_map",
    };

    // static void Main(string[] args)
    // {
    // }

    public static void SearchStrings()
    {

        string directoryPath = paths[0];

        if (!Directory.Exists(directoryPath))
        {
            Console.WriteLine($"Directory not found: {directoryPath}");
            return;
        }

        var cppFiles = Directory.GetFiles(directoryPath, "*.cpp", SearchOption.AllDirectories);
        var hFiles = Directory.GetFiles(directoryPath, "*.h", SearchOption.AllDirectories);

        var textFiles = cppFiles.Union(hFiles).ToArray();
        var results = new List<string>();

        foreach (var file in textFiles)
        {
            var lines = File.ReadAllLines(file);
            for (int lineNumber = 0; lineNumber < lines.Length; lineNumber++)
            {
                foreach (var searchString in searchStrings)
                {
                    if (lines[lineNumber].Contains(searchString, StringComparison.OrdinalIgnoreCase))
                    {
                        results.Add($"{file} (Line {lineNumber + 1}): {lines[lineNumber]}");
                    }
                }
            }
        }

        if (results.Count == 0)
        {
            Console.WriteLine("No matches found.");
        }
        else
        {
            Console.WriteLine("Matches found:");
            foreach (var result in results)
            {
                Console.WriteLine(result);
            }
        }
    }
}