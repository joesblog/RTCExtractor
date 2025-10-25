

using System.Buffers.Binary;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;


namespace ConsoleApp
{



  internal class Program
  {

    public static   int Main(string[] args)
    {
      System.Diagnostics.Debug.WriteLine(
  $"Proc bitness: {(Environment.Is64BitProcess ? "x64" : "x86")}. We must be 32 bit (x86)! ");
      if (args.Length < 2)
      {
        Console.Error.WriteLine("Usage: RTCExtractor   <input_blob> <out_dir>");
        return 1;
      }

      string inPath = args[0], outDir = args[1];

      if (!File.Exists(inPath)) { Console.Error.WriteLine("Input not found."); return 1; }
      Directory.CreateDirectory(outDir);

      byte[] data = ModHandling.GetModData(inPath);
      Console.WriteLine($"Scanning {inPath} ({data.Length} bytes)");
      Console.WriteLine($"{"Kind",-12} {"Start",10} {"End",10} {"Size",10}  Name");

      // Load built-in rules
      var rules = Extractor.BuiltInRules();

      // PASS 1: detect all hits with the generic engine
      var hits = Extractor.DetectAll(data, rules);
      hits.Sort((a, b) => a.Start.CompareTo(b.Start));

      // Tail string pool (after last hit)
      int tailStart = hits.Count > 0 ? Extractor.MaxEnd(hits) : data.Length;
      var tailPool = Extractor.BuildTailStringPool(data, tailStart, 4);

      // PASS 2: name + write
      foreach (var h in hits)
      {
        string baseName = Extractor.ResolveName(data, h, tailPool);
        if (string.IsNullOrEmpty(baseName)) baseName = $"{Extractor.Sanitize(h.Rule.Kind.Replace('/', '_'))}_{h.Start:000000}";
        //string fileName = Extractor.Sanitize(baseName) + h.Rule.Extension;
        string fileName = baseName + h.Rule.Extension;
        string outPath = Extractor.UniquePath(Path.Combine(outDir, fileName));
        File.WriteAllBytes(outPath, Extractor.Slice(data, h.Start, h.Size));
        Console.WriteLine($"{h.Rule.Kind,-12} {h.Start,10} {h.End,10} {h.Size,10}  {Path.GetFileName(outPath)}");
      }


      Console.ReadLine();
      return 0;
    }
  }

  public class ModHandling
  {

    const string DLL = "RTCData.dll";
    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
    public static   extern IntPtr test();


    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    internal  static  extern int GetModSection(string inPath, out IntPtr data, out UIntPtr size);

    [DllImport(DLL, CallingConvention = CallingConvention.Cdecl)]
    internal  static  extern void FreeBuffer(IntPtr p);

    public static   byte[] GetModData(string path, string section = "MOD")
    {
      IntPtr ptr;
      UIntPtr usize;
      int hr = GetModSection(path, out ptr, out usize);
      byte[] res = null;
      if (hr != 0)
      {
        Console.WriteLine($"GetModSection failed: {hr}");
        return null;
      }

      try
      {
        ulong len = usize.ToUInt64();
        if (len == 0 || ptr == IntPtr.Zero)
        {
          Console.WriteLine("Empty section.");
          return null;

        }

        if (len > int.MaxValue)
          throw new OverflowException("Section too large to fit in a managed array.");

        byte[] data = new byte[(int)len];
        Marshal.Copy(source: ptr, destination: data, startIndex: 0, length: (int)len);
        res = data;
        // TODO: use `data` as needed
        Console.WriteLine($"Got {data.Length} bytes from MOD section.");
      }
      finally
      {
        FreeBuffer(ptr);
      }
      return res;
    }
  }

  public class Extractor
  {
    #region models
    public enum BoundaryKind { SizeField, Terminator, ChunkWalk,Custom }

    public sealed class SizeFieldSpec { public int Offset; public int Bytes; public string Endian = "LE"; public int Adjust; }
    public sealed class TerminatorSpec { public byte[] EndPattern = Array.Empty<byte>(); }
    public sealed class ChunkWalkSpec { public int LenBytes; public string LenEndian = "BE"; public int TypeBytes; public int CrcBytes; public byte[] EndType = Array.Empty<byte>(); }

    public sealed class ValueCheck { public int RelativeOffset; public int Bytes; public string Endian = "LE"; public int[] AllowedValues = Array.Empty<int>(); }

   public  sealed class Rule
    {
      public string Name = "";
      public string Kind = "";
      public byte[] StartPattern = Array.Empty<byte>();
      public string Extension = ".bin";
      public BoundaryKind Boundary;
      public int AnchorOffset = 0;
      public SizeFieldSpec? SizeField;
      public TerminatorSpec? Terminator;
      public ChunkWalkSpec? ChunkWalk;
      public int MinSize = 16;
      public int MaxSize = int.MaxValue;
      public ValueCheck[]? Validations;
      public string[]? PreferredPrefixes;
      public int NearbyWindow = 256;
    }

    public sealed class Hit
    {
      public Rule Rule;
      public int Start;
      public int Size;
      public int End => Start + Size;
      public Hit(Rule r, int s, int z) { Rule = r; Start = s; Size = z; }
    }
    #endregion
    #region utils
    public static  int IndexOf(byte[] hay, byte[] needle, int start)
    {
      if (needle.Length == 0) return start;
      for (int i = start; i <= hay.Length - needle.Length; i++)
      {
        if (hay[i] != needle[0]) continue;
        int j = 1;
        for (; j < needle.Length; j++)
          if (hay[i + j] != needle[j]) break;
        if (j == needle.Length) return i;
      }
      return -1;
    }

    public static  bool EqualsBytes(byte[] data, int offset, byte[] rhs)
    {
      if (offset < 0 || offset + rhs.Length > data.Length) return false;
      for (int i = 0; i < rhs.Length; i++)
        if (data[offset + i] != rhs[i]) return false;
      return true;
    }

    public static  byte[] Slice(byte[] data, int start, int len)
    {
      var buf = new byte[len];
      Buffer.BlockCopy(data, start, buf, 0, len);
      return buf;
    }

    public static  string UniquePath(string path)
    {
      if (!File.Exists(path)) return path;
      string dir = Path.GetDirectoryName(path)!;
      string stem = Path.GetFileNameWithoutExtension(path);
      string ext = Path.GetExtension(path);
      for (int i = 1; i < 10000; i++)
      {
        string cand = Path.Combine(dir, $"{stem}_{i}{ext}");
        if (!File.Exists(cand)) return cand;
      }
      return path;
    }

    public static  string Sanitize(string s)
    {
      var sb = new StringBuilder(s.Length);
      foreach (var c in s) sb.Append(char.IsLetterOrDigit(c) || c == '_' || c == '-' ? c : '_');
      if (sb.Length > 0 && sb[0] == '.') sb[0] = '_';
      return sb.ToString();
    }

    public static  int MaxEnd(List<Hit> hits)
    {
      int m = 0;
      for (int i = 0; i < hits.Count; i++)
      {
        int e = hits[i].End;
        if (e > m) m = e;
      }
      return m;
    }

    static int ReadUInt16BE(byte[] data, int off)
        => (off >= 0 && off + 2 <= data.Length) ? (data[off] << 8) | data[off + 1] : 0;

    static string GetAscii(byte[] data, int off, int len)
        => (off >= 0 && off + len <= data.Length) ? Encoding.ASCII.GetString(data, off, len) : "";
    #endregion

    #region detection
    public   static List<Hit> DetectAll(byte[] data, List<Rule> rules)
    {
      var hits = new List<Hit>(128);
      foreach (var r in rules)
      {
        if (r.StartPattern.Length == 0) continue; // never allow empty patterns

        int searchIdx = 0;
        while (true)
        {
          int patPos = IndexOf(data, r.StartPattern, searchIdx);
          if (patPos < 0) break;

          // Real file start is pattern position + anchor (e.g., MOD: pat at +1080)
          int start = patPos + r.AnchorOffset;
          if (start < 0 || start >= data.Length)
          {
            searchIdx = patPos + 1;   // advance past the pattern
            continue;
          }

          int size = r.Boundary switch
          {
            BoundaryKind.SizeField => ComputeEndBySizeField(data, start, r),
            BoundaryKind.Terminator => ComputeEndByTerminator(data, start, r),
            BoundaryKind.ChunkWalk => ComputeEndByChunkWalk(data, start, r),
            BoundaryKind.Custom => ComputeEndByCustom(data, start, r),
            _ => -1
          };

          if (size > 0 &&
              size >= r.MinSize &&
              size <= r.MaxSize &&
              start + size <= data.Length &&
              PassesValidations(data, start, r))
          {
            hits.Add(new Hit(r, start, size));
            // Advance past the *pattern* we matched to avoid re-finding it
            searchIdx = patPos + r.StartPattern.Length;
          }
          else
          {
            searchIdx = patPos + 1;
          }
        }
      }
      return hits;
    }
    static int ComputeEndByCustom(byte[] data, int start, Rule r)
    {
      if (r.Kind.Equals("MOD", StringComparison.OrdinalIgnoreCase))
        return ComputeEndByAmigaMod(data, start);
      return -1;
    }
    static int ComputeEndByAmigaMod(byte[] data, int start)
    {
      if (start + 1084 > data.Length) return -1;

      string sig = GetAscii(data, start + 1080, 4);
      int ch = ChannelsFromModSignature(sig);
      if (ch <= 0) return -1;

      const int TitleLen = 20;
      const int SampleHdrs = 31;
      const int SampleHdrSize = 30;

      int sampleHeadersOff = start + TitleLen;
      int songLengthOff = start + TitleLen + SampleHdrs * SampleHdrSize; // 950
      int patternTableOff = songLengthOff + 2;                              // 952

      if (songLengthOff + 1 > data.Length) return -1;
      int songLength = data[songLengthOff];
      if (songLength <= 0 || songLength > 128) return -1;

      if (patternTableOff + 128 > data.Length) return -1;
      int maxPat = 0;
      for (int i = 0; i < songLength; i++)
      {
        int v = data[patternTableOff + i];
        if (v > maxPat) maxPat = v;
      }
      int patternCount = maxPat + 1;
      if (patternCount <= 0 || patternCount > 256) return -1;

      long patternSize = 64L * ch * 4;
      long patternsBytes = patternCount * patternSize;

      if (sampleHeadersOff + SampleHdrs * SampleHdrSize > data.Length) return -1;
      long sampleBytes = 0;
      for (int i = 0; i < SampleHdrs; i++)
      {
        int hdr = sampleHeadersOff + i * SampleHdrSize;
        int lenWords = ReadUInt16BE(data, hdr + 22);
        int vol = data[hdr + 25];
        if (vol < 0 || vol > 64) return -1;
        sampleBytes += (long)lenWords * 2;
      }

      long total = 1084L + patternsBytes + sampleBytes;
      if (total <= 0 || start + total > data.Length) return -1;
      if (patternsBytes < patternSize || patternsBytes > data.Length) return -1;

      return (int)total;
    }

    static int ChannelsFromModSignature(string sig)
    {
      if (sig is "M.K." or "M!K!" or "M&K!" or "FLT4") return 4;
      if (sig == "4CHN") return 4;
      if (sig == "6CHN") return 6;
      if (sig is "8CHN" or "FLT8") return 8;
      if (sig.Length == 4 && char.IsDigit(sig[0]) && char.IsDigit(sig[1]) && sig[2] == 'C' && sig[3] == 'H')
      {
        int n = (sig[0] - '0') * 10 + (sig[1] - '0');
        if (n >= 4 && n <= 32) return n;
      }
      return -1;
    }

    public static  List<Hit> DetectAll2(byte[] data, List<Rule> rules)
    {
      var hits = new List<Hit>(128);
      for (int ri = 0; ri < rules.Count; ri++)
      {
        var r = rules[ri];
        int i = 0;
        while (true)
        {
          int start = IndexOf(data, r.StartPattern, i);
          if (start < 0) break;

          int size = r.Boundary switch
          {
            BoundaryKind.SizeField => ComputeEndBySizeField(data, start, r),
            BoundaryKind.Terminator => ComputeEndByTerminator(data, start, r),
            BoundaryKind.ChunkWalk => ComputeEndByChunkWalk(data, start, r),
            //BoundaryKind.Custom => ComputeEndByCustom(data, start, r),
            _ => -1
          };

          if (size > 0 &&
              size >= r.MinSize &&
              size <= r.MaxSize &&
              start + size <= data.Length &&
              PassesValidations(data, start, r))
          {
            hits.Add(new Hit(r, start, size));
            i = start + size; // skip the payload
          }
          else
          {
            i = start + 1; // slide forward
          }
        }
      }
      return hits;
    }

    public static  int ComputeEndBySizeField(byte[] data, int start, Rule r)
    {
      var s = r.SizeField; if (s == null) return -1;
      int off = start + s.Offset;
      if (off < 0 || off + s.Bytes > data.Length) return -1;

      ulong value;
      if (s.Bytes == 2)
        value = s.Endian == "LE" ? BinaryPrimitives.ReadUInt16LittleEndian(data.AsSpan(off, 2))
                                 : BinaryPrimitives.ReadUInt16BigEndian(data.AsSpan(off, 2));
      else if (s.Bytes == 4)
        value = s.Endian == "LE" ? BinaryPrimitives.ReadUInt32LittleEndian(data.AsSpan(off, 4))
                                 : BinaryPrimitives.ReadUInt32BigEndian(data.AsSpan(off, 4));
      else return -1;

      long total = (long)value + s.Adjust;
      return total > 0 && total <= int.MaxValue ? (int)total : -1;
    }

    public static  int ComputeEndByTerminator(byte[] data, int start, Rule r)
    {
      var t = r.Terminator; if (t == null) return -1;
      int pos = IndexOf(data, t.EndPattern, start + r.StartPattern.Length);
      if (pos < 0) return -1;
      return (pos + t.EndPattern.Length) - start;
    }

    public static  int ComputeEndByChunkWalk(byte[] data, int start, Rule r)
    {
      var spec = r.ChunkWalk; if (spec == null) return -1;
      int p = start + r.StartPattern.Length;
      while (p + spec.LenBytes + spec.TypeBytes + spec.CrcBytes <= data.Length)
      {
        if (p + spec.LenBytes > data.Length) return -1;
        uint clen = spec.LenEndian == "BE"
            ? BinaryPrimitives.ReadUInt32BigEndian(data.AsSpan(p, 4))
            : BinaryPrimitives.ReadUInt32LittleEndian(data.AsSpan(p, 4));

        if (clen > int.MaxValue) return -1;
        int next = p + spec.LenBytes + spec.TypeBytes + (int)clen + spec.CrcBytes;
        if (next > data.Length) return -1;

        // chunk type field right after length
        if (!EqualsBytes(data, p + spec.LenBytes, spec.EndType))
        {
          p = next;
          continue;
        }
        // EndType encountered: return inclusive size
        return next - start;
      }
      return -1;
    }

    public static  bool PassesValidations(byte[] data, int start, Rule r)
    {
      var vals = r.Validations; if (vals == null) return true;
      for (int i = 0; i < vals.Length; i++)
      {
        var v = vals[i];
        int off = start + v.RelativeOffset;
        if (off < 0 || off + v.Bytes > data.Length) return false;
        int val;
        if (v.Bytes == 2)
          val = v.Endian == "LE" ? BinaryPrimitives.ReadUInt16LittleEndian(data.AsSpan(off, 2))
                                 : BinaryPrimitives.ReadUInt16BigEndian(data.AsSpan(off, 2));
        else if (v.Bytes == 4)
          val = v.Endian == "LE" ? BinaryPrimitives.ReadInt32LittleEndian(data.AsSpan(off, 4))
                                 : BinaryPrimitives.ReadInt32BigEndian(data.AsSpan(off, 4));
        else return false;

        bool ok = false;
        for (int k = 0; k < v.AllowedValues.Length; k++)
          if (val == v.AllowedValues[k]) { ok = true; break; }
        if (!ok) return false;
      }
      return true;
    }
    #endregion

    #region naming
    public static  string ResolveName(byte[] data, Hit h, HashSet<string> tailPool)
    {
      string neighbor = FindLastAsciiZBefore(data, h.Start, h.Rule.NearbyWindow);

      // Prefer prefixes if configured
      if (!string.IsNullOrEmpty(neighbor) && h.Rule.PreferredPrefixes != null)
      {
        for (int i = 0; i < h.Rule.PreferredPrefixes.Length; i++)
        {
          var pref = h.Rule.PreferredPrefixes[i];
          if (neighbor.StartsWith(pref, StringComparison.OrdinalIgnoreCase))
            return neighbor;
        }
      }
      if (!string.IsNullOrEmpty(neighbor)) return neighbor;

      if (h.Rule.PreferredPrefixes != null && tailPool.Count > 0)
      {
        foreach (var pref in h.Rule.PreferredPrefixes)
        {
          foreach (var s in tailPool)
          {
            if (s.StartsWith(pref, StringComparison.OrdinalIgnoreCase))
              return s;
          }
        }
      }
      return string.Empty;
    }

    public static  string FindLastAsciiZBefore(byte[] data, int offset, int window)
    {
      int start = Math.Max(0, offset - window);
      int end = offset;
      int bestS = -1, bestLen = 0;

      int i = start;
      while (i < end)
      {
        while (i < end && !IsPrintable(data[i])) i++;
        int s = i;
        while (i < end && IsPrintable(data[i])) i++;
        int e = i;
        if (s < e && i < end && data[i] == 0)
        {
          int len = e - s;
          if (len >= 4) { bestS = s; bestLen = len; }
          while (i < end && data[i] == 0) i++;
        }
      }
      return bestLen > 0 ? Encoding.ASCII.GetString(data, bestS, bestLen) : string.Empty;
    }

    public static  bool IsPrintable(byte b) => b >= 32 && b <= 126;

    public static  HashSet<string> BuildTailStringPool(byte[] data, int tailStart, int minLen)
    {
      var set = new HashSet<string>(StringComparer.Ordinal);
      int i = Math.Max(0, tailStart);
      while (i < data.Length)
      {
        if (IsPrintable(data[i]))
        {
          int s = i;
          while (i < data.Length && IsPrintable(data[i])) i++;
          int e = i;
          if (i < data.Length && data[i] == 0 && e - s >= minLen)
            set.Add(Encoding.ASCII.GetString(data, s, e - s));
          while (i < data.Length && data[i] == 0) i++;
        }
        else i++;
      }
      return set;
    }
    #endregion

    public static  List<Rule> BuiltInRules()
    {
      var list = new List<Rule>();

      // BMP: "BM", size at +2 LE, DIB size at +14 in {12,40,108,124}
      list.Add(new Rule
      {
        Name = "BMP",
        Kind = "BMP",
        StartPattern = Encoding.ASCII.GetBytes("BM"),
        Extension = ".bmp",
        Boundary = BoundaryKind.SizeField,
        SizeField = new SizeFieldSpec { Offset = 2, Bytes = 4, Endian = "LE", Adjust = 0 },
        Validations = new[] { new ValueCheck { RelativeOffset = 14, Bytes = 4, Endian = "LE", AllowedValues = new[] { 12, 40, 108, 124 } } },
        PreferredPrefixes = new[] { "BITMAP_", "ICON_", "MAP_", "WALL_", "UI_", "SPR_" },
        NearbyWindow = 256
      });

      // RIFF: "RIFF", size at +4 LE, total = size + 8; prefer WAVE
      list.Add(new Rule
      {
        Name = "RIFF",
        Kind = "RIFF/WAVE",
        StartPattern = Encoding.ASCII.GetBytes("RIFF"),
        Extension = ".wav",
        Boundary = BoundaryKind.SizeField,
        SizeField = new SizeFieldSpec { Offset = 4, Bytes = 4, Endian = "LE", Adjust = 8 },
        PreferredPrefixes = new[] { "SOUND_", "FX_", "VOICE_", "MUSIC_" },
        NearbyWindow = 256
      });
      foreach (var sig in new[] { "M.K.", "M!K!", "M&K!", "4CHN", "6CHN", "8CHN", "FLT4", "FLT8" })
      {
        list.Add(new Rule
        {
          Name = "MOD",
          Kind = "MOD",
          StartPattern = Encoding.ASCII.GetBytes(sig),
          AnchorOffset = -1080,                 // pattern lives at start+1080
          Extension = ".mod",
          Boundary = BoundaryKind.Custom,       // ComputeEndByCustom -> ComputeEndByAmigaMod
          MinSize = 2048,
          PreferredPrefixes = new[] { "MUSIC_", "TRACK_", "MOD_" },
          NearbyWindow = 256
        });
      }
      // PNG: 8-byte sig, chunk-walk until IEND
      //list.Add(new Rule
      //{
      //  Name = "PNG",
      //  Kind = "PNG",
      //  StartPattern = new byte[] { 0x89, (byte)'P', (byte)'N', (byte)'G', 0x0D, 0x0A, 0x1A, 0x0A },
      //  Extension = ".png",
      //  Boundary = BoundaryKind.ChunkWalk,
      //  ChunkWalk = new ChunkWalkSpec { LenBytes = 4, LenEndian = "BE", TypeBytes = 4, CrcBytes = 4, EndType = Encoding.ASCII.GetBytes("IEND") },
      //  PreferredPrefixes = new[] { "BITMAP_", "ICON_", "SPR_", "UI_" },
      //  NearbyWindow = 256
      //});

      //// JPEG: SOI..EOI
      //list.Add(new Rule
      //{
      //  Name = "JPEG",
      //  Kind = "JPEG",
      //  StartPattern = new byte[] { 0xFF, 0xD8, 0xFF },
      //  Extension = ".jpg",
      //  Boundary = BoundaryKind.Terminator,
      //  Terminator = new TerminatorSpec { EndPattern = new byte[] { 0xFF, 0xD9 } },
      //  PreferredPrefixes = new[] { "BITMAP_", "ICON_", "SPR_", "UI_" },
      //  NearbyWindow = 256
      //});

      //// GIF 89a
      //list.Add(new Rule
      //{
      //  Name = "GIF89a",
      //  Kind = "GIF",
      //  StartPattern = Encoding.ASCII.GetBytes("GIF89a"),
      //  Extension = ".gif",
      //  Boundary = BoundaryKind.Terminator,
      //  Terminator = new TerminatorSpec { EndPattern = new byte[] { 0x3B } },
      //  PreferredPrefixes = new[] { "BITMAP_", "ICON_", "SPR_", "UI_" },
      //  NearbyWindow = 256
      //});
      //// GIF 87a
      //list.Add(new Rule
      //{
      //  Name = "GIF87a",
      //  Kind = "GIF",
      //  StartPattern = Encoding.ASCII.GetBytes("GIF87a"),
      //  Extension = ".gif",
      //  Boundary = BoundaryKind.Terminator,
      //  Terminator = new TerminatorSpec { EndPattern = new byte[] { 0x3B } },
      //  PreferredPrefixes = new[] { "BITMAP_", "ICON_", "SPR_", "UI_" },
      //  NearbyWindow = 256
      //});

      return list;
    }
  }
}
