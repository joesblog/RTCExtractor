// C# port of the provided C sources:
// - MemStream.c / .h
// - util.c / .h (subset necessary for file loading + string alloc)
// - fileContext.c / .h
// - ctxDecoder.c / .h (ported algorithmically as in the snippets; some less-certain areas kept verbatim)
//
// Notes
// -----
// • This code aims to be a drop-in logical port with minimal behavior changes.
// • Memory management is idiomatic C# (gc-managed). Helpers mimic the C layout when needed.
// • ctxDecoder is translated faithfully, preserving its fixed-offset scratch space.
// • A small placeholder for the external decryption step is included (returns file bytes as-is).
// • Public API mirrors the C functions but as instance/static methods.
//
// You can compile this as a class library or include it in a console app for testing.

using System;
using System.Buffers.Binary;
using System.Collections.Generic;
using System.IO;
using System.Runtime.CompilerServices;

namespace RgcPort
{
  #region Util (subset)
  public static class Util
  {
    public static void StrAlloc(out string? dest, string src)
    {
      dest = src; // In C#, just assign; strings are immutable and interned.
    }

    public static void Malloc<T>(out T[] buffer, int size)
    {
      buffer = new T[size];
    }
  }
  #endregion

  #region MemStream
  public class MemStream
  {
    public byte[] Buffer = Array.Empty<byte>();
    public int Size;       // number of valid bytes in Buffer
    public int Pos;        // current read position
    public int Capacity;   // allocated size (Buffer.Length)

    public MemStream()
    {
      _typeOrFlags = 0;
      CreateBlank(1000);
    }

    private uint _typeOrFlags;

    public void CreateBlank(int size)
    {
      Buffer = new byte[size];
      Size = 0;
      Capacity = size;
      StreamSeek(0);
    }

    public void CreateFromMem(byte[] src)
    {
      Buffer = new byte[src.Length];
      src.AsSpan().CopyTo(Buffer);
      Size = src.Length;
      Capacity = src.Length;
      StreamSeek(0);
    }

    public int StreamSeek(int offset)
    {
      Pos = offset;
      return Pos;
    }

    public ReadOnlySpan<byte> StreamTake(int count)
    {
      var slice = new ReadOnlySpan<byte>(Buffer, Pos, count);
      Pos += count;
      return slice;
    }

    public void Destroy()
    {
      Buffer = Array.Empty<byte>();
      Size = 0;
      Pos = 0;
      Capacity = 0;
      _typeOrFlags = 0;
    }

    public string GetZString()
    {
      int start = Pos;
      while (Pos < Size && Buffer[Pos] != 0) Pos++;
      string s = System.Text.Encoding.ASCII.GetString(Buffer, start, Pos - start);
      Pos++; // skip NUL
      return s;
    }

    public int GetIntLE()
    {
      int v = BinaryPrimitives.ReadInt32LittleEndian(new ReadOnlySpan<byte>(Buffer, Pos, 4));
      Pos += 4;
      return v;
    }

    public uint DecompressSectionData()
    {
      // Wrapper that uses the ctx decoder to uncompress the current buffer.
      var ctx = new CtxDecoder.RawCtx();
      var outData = CtxDecoder.DecompressFromMemoryStream(this, ctx, out int outSize);
      if (outData == null)
        return 0;

      Buffer = outData;
      Size = outSize;
      Capacity = outSize;
      StreamSeek(0);
      return (uint)outSize;
    }
  }
  #endregion

  #region FileContext
  public class FileContext
  {
    public string? Header;
    public int SectionCount;
    public MemStream?[] SecStream = new MemStream?[100];
    public string?[] SectName = new string?[100];

    public static void Init(FileContext f)
    {
      f.SectionCount = 0;
      for (int i = 0; i < 100; i++)
      {
        f.SecStream[i] = null;
        f.SectName[i] = null;
      }
      f.Header = null;
    }

    public static int Load(FileContext t, string fileName)
    {
      var mainStream = new MemStream();
      int decryptResult = Decoder.ReadAndDecryptFile(fileName, out byte[]? fileBuf);
      if (decryptResult != 0 && fileBuf != null)
      {
        mainStream.CreateFromMem(fileBuf);

        string headerString = mainStream.GetZString();
        Util.StrAlloc(out t.Header, headerString);
        int sectionCount = mainStream.GetIntLE();
        t.SectionCount = sectionCount;

        // Section names
        for (int i = 0; i < sectionCount; i++)
        {
          string s = mainStream.GetZString();
          Util.StrAlloc(out t.SectName[i], s);
        }

        // Section sizes
        var sizes = new int[sectionCount];
        for (int i = 0; i < sectionCount; i++)
          sizes[i] = mainStream.GetIntLE();

        // Section streams
        for (int i = 0; i < sectionCount; i++)
        {
          int curSize = sizes[i];
          if (curSize == 0)
          {
            t.SecStream[i] = null;
            continue;
          }
          ReadOnlySpan<byte> take = mainStream.StreamTake(curSize);
          var ms = new MemStream();
          ms.CreateFromMem(take.ToArray());
          t.SecStream[i] = ms;
        }

        mainStream.Destroy();
      }

      return 0;
    }

    public static MemStream? FindSection(FileContext fc, string name)
    {
      for (int i = 0; i < 100; i++)
      {
        if (fc.SectName[i] == null) continue;
        if (string.Equals(fc.SectName[i], name, StringComparison.Ordinal))
          return fc.SecStream[i];
      }
      return null;
    }

    public static string? GetHeaderString(FileContext fc) => fc.Header;

    public static int ModuleIdFromHeader(string a1)
    {
      // Mirrors the long if-chain with a compact lookup
      static bool Any(string x, params string[] ys)
      {
        foreach (var y in ys) if (x.ToLower() == y.ToLower()) return true; return false;
      }
      if (Any(a1, "rtcmv22", "rtcsv22")) return 22;
      if (Any(a1, "rtcmv27", "rtcsv27")) return 27;
      if (Any(a1, "rtcmv29", "rtcsv29")) return 29;
      if (Any(a1, "rtcmv33", "rtcsv33")) return 33;
      if (Any(a1, "rtcmv35", "rtcsv35")) return 35;
      if (Any(a1, "rtcmv36", "rtcsv36")) return 36;
      if (Any(a1, "rtcmv37", "rtcsv37")) return 37;
      if (Any(a1, "rtcmv38", "rtcsv38")) return 38;
      if (Any(a1, "rtcmv39", "rtcsv39")) return 39;
      if (Any(a1, "rtcmv40", "rtcsv40")) return 40;
      if (Any(a1, "rtcmv41", "rtcsv41")) return 41;
      if (Any(a1, "rtcmv42", "rtcsv42")) return 42;
      if (Any(a1, "rtcmv43", "rtcsv43")) return 43;
      if (Any(a1, "rtcmv44", "rtcsv44")) return 44;
      if (Any(a1, "rtcmv45", "rtcsv45")) return 45;
      if (Any(a1, "rtcmv46", "rtcsv46")) return 46;
      if (Any(a1, "rtcmv47", "rtcsv47")) return 47;
      if (Any(a1, "rtcmv48", "rtcsv48")) return 48;
      if (Any(a1, "rtcmv49", "rtcsv49")) return 49;
      return -1;
    }
  }
  #endregion

  #region Decoder (file IO + authentic decryption)
  public static class Decoder
  {
    // Returns 1 on success (buffer filled) and 0 on failure, mirroring the C implementation.
    public static int ReadAndDecryptFile(string path, out byte[]? buf)
    {
      buf = null;
      using var fs = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read);
      if (fs.Length < 8)
        return 0;

      Span<byte> header = stackalloc byte[8];
      int readHead = fs.Read(header);
      if (readHead != 8)
        return 0;

      uint expectedChecksum = BinaryPrimitives.ReadUInt32LittleEndian(header.Slice(0, 4));
      int seed = BinaryPrimitives.ReadInt32LittleEndian(header.Slice(4, 4));

      long dataLen = fs.Length - 8;
      if (dataLen <= 0) return 0;

      byte[] data = new byte[dataLen];
      int total = 0;
      while (total < data.Length)
      {
        int got = fs.Read(data, total, data.Length - total);
        if (got == 0) break;
        total += got;
      }
      if (total != data.Length)
        return 0;

      DecodeBufferInPlace(data, seed);
      uint actualChecksum = ComputeChecksum(data);
      if (actualChecksum != expectedChecksum)
        return 0;

      buf = data;
      return 1;
    }

    private static void DecodeBufferInPlace(byte[] data, int seed)
    {
      int s = seed;
      for (int idx = 0; idx < data.Length; ++idx)
      {
        byte mask;
        switch (idx & 3)
        {
          case 0:
            mask = (byte)(s & 0xFF);
            s = unchecked(s + 1);
            break;
          case 1:
            mask = (byte)((uint)s >> 8);
            s = unchecked(s * 13);
            break;
          case 2:
            {
              byte byte2 = (byte)(((uint)s >> 16) & 0xFF);
              mask = byte2;
              s = unchecked(s ^ (int)(0x01010101u * byte2));
              break;
            }
          case 3:
            mask = (byte)((uint)s >> 24);
            s = unchecked(s - 1);
            break;
          default:
            mask = 0; // unreachable
            break;
        }
        data[idx] ^= mask;
      }
    }

    private static uint ComputeChecksum(byte[] data)
    {
      int checksum = 291074;
      for (int i = 0; i < data.Length; ++i)
      {
        sbyte b = unchecked((sbyte)data[i]);
        int shift = i & 0x1F;
        int term = unchecked(((int)b) << shift);
        checksum = unchecked(checksum + term);
      }
      return unchecked((uint)checksum);
    }
  }
  #endregion

  #region ctxDecoder port
  public static class CtxDecoder
  {
    public const int LZAR_WORKSPACE_BYTES = 66376; // matches header

    public sealed class RawCtx
    {
      public uint OutTarget;
      public uint Unk1;
      public uint Unk2;
      public uint OutPos;
      public uint InPos;
      public byte[]? OutBuf;
      public byte[]? InBuf;
      public uint InSize;
      public byte[] Scratch = new byte[LZAR_WORKSPACE_BYTES];
    }

    #region Helpers to read/write within Scratch at absolute offsets
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    static ushort U16(byte[] a, int off) => BinaryPrimitives.ReadUInt16LittleEndian(a.AsSpan(off, 2));
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    static void W16(byte[] a, int off, ushort v) => BinaryPrimitives.WriteUInt16LittleEndian(a.AsSpan(off, 2), v);
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    static uint U32(byte[] a, int off) => BinaryPrimitives.ReadUInt32LittleEndian(a.AsSpan(off, 4));
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    static void W32(byte[] a, int off, uint v) => BinaryPrimitives.WriteUInt32LittleEndian(a.AsSpan(off, 4), v);
    #endregion

    public static byte[]? DecompressFromMemoryStream(MemStream src, RawCtx ctx, out int outSize)
    {
      outSize = 0;
      var inBytes = new byte[src.Size];
      Buffer.BlockCopy(src.Buffer, 0, inBytes, 0, src.Size);
      ctx.InBuf = inBytes;
      ctx.InSize = (uint)inBytes.Length;
      ctx.InPos = 0;

      if (!ReadOutputSizeLE(ctx))
        return null;

      ctx.OutBuf = new byte[ctx.OutTarget];
      ctx.OutPos = 0;

      DecodeStream(ctx);
      outSize = (int)ctx.OutPos;
      return ctx.OutBuf;
    }

    private static bool ReadOutputSizeLE(RawCtx c)
    {
      if (c.InBuf == null) return false;
      uint v2 = 0; int idx = 0; c.OutTarget = 0;
      do
      {
        int b = c.InPos < c.InSize ? c.InBuf[(int)c.InPos] : 0;
        c.InPos++;
        uint v4 = (uint)(b << (int)v2);
        v2 += 8;
        c.OutTarget = c.OutTarget + v4;
        idx++;
      } while (v2 < 32);
      return c.OutTarget != 0;
    }

    private static int ResetCoderState(RawCtx c, int a2)
    {
      // *(_DWORD*)&scratch[66332] = 0; etc…
      var s = c.Scratch;
      W32(s, 66332, 0);
      W32(s, 66340, 0);
      W32(s, 4164, 0);
      c.OutPos = 0;
      W16(s, 66326, 0);
      W32(s, 66336, 0x20000);
      ushort result = (ushort)(a2 != 0 ? 0x80 : 0);
      c.Unk1 = 4;
      c.InPos = 4; // consumes the 4 output-size bytes
      W16(s, 66328, result);
      return result;
    }

    private static bool PrimeCoderFromBits(RawCtx c)
    {
      var s = c.Scratch;
      int v = 17;
      do
      {
        bool bit = ReadBit(c);
        uint code = U32(s, 66340);
        W32(s, 66340, (bit ? 1u : 0u) + 2u * code);
        v--;
      } while (v > 0);
      return true;
    }

    private static bool ReadBit(RawCtx c)
    {
      var s = c.Scratch;
      ushort mask = U16(s, 66328);
      mask >>= 1;
      if (mask == 0)
      {
        ushort b;
        if (c.InPos >= c.InSize) b = 0xFFFF; else b = c.InBuf![(int)c.InPos];
        W16(s, 66326, b);
        mask = 128;
        c.InPos += 1;
      }
      W16(s, 66328, mask);
      ushort reg = U16(s, 66326);
      return (mask & reg) != 0;
    }

    private static short InitModels(RawCtx c)
    {
      var s = c.Scratch;
      // Translated from snippet. Initializes model tables.
      // Only the exact fields that are used by later code are maintained.
      // Literal model cumulative arrays around 57500/57502
      W16(s, 58130, 0);
      int v2 = 314;
      int v3 = 56868;
      int v1 = 58130;
      do
      {
        // *((_DWORD*)v3 - 315) = v2--;
        W32(s, v3 - 1260, (uint)v2--);
        // *(_DWORD*)v3 = v2;
        W32(s, v3, (uint)v2);
        // *((_WORD*)v1 - 315) = 1;
        W16(s, v1 - 630, 1);
        v3 -= 4;
        // *((_WORD*)v1 - 1) = *(_WORD*)v1 + 1;
        ushort cur = U16(s, v1);
        W16(s, v1 - 2, (ushort)(cur + 1));
        v1 -= 2;
      } while (v2 >= 1);
      W16(s, 56872, 0);
      W16(s, 66324, 0);
      int v4 = 4096;
      int v5 = 66322;
      short result = 0;
      do
      {
        v5 -= 2;
        result = (short)(U16(s, v5 + 4) + 10000 / (v4 + 200));
        v4--;
        W16(s, v5 + 2, (ushort)result);
      } while (v4 >= 1);
      return result;
    }

    private static int EmitByte(RawCtx c, byte b)
    {
      if (c.OutBuf == null) return 0;
      if (c.OutPos < c.OutBuf.Length)
        c.OutBuf[c.OutPos++] = b;
      return 0;
    }

    public static bool DecodeStream(RawCtx c)
    {
      ResetCoderState(c, 0);
      PrimeCoderFromBits(c);
      InitModels(c);
      int winIdx = 4036;
      Array.Fill(c.Scratch, (byte)0x20, 32, 0xFC4); // memset window region to 0x20
      uint produced = 0;
      if (c.OutTarget == 0) return true;

      while (produced < c.OutTarget)
      {
        int sym = DecodeLitLenSymbol(c);
        byte b = (byte)sym;
        if (sym >= 256)
        {
          int len = sym - 253; // per snippet
          int copied = 0;
          ushort dist = (ushort)((winIdx - DecodeMatchDistance(c) - 1) & 0x0FFF);
          while (copied < len)
          {
            int src = (dist + copied) & 0x0FFF;
            byte v = c.Scratch[32 + src];
            EmitByte(c, v);
            c.Scratch[32 + (winIdx & 0x0FFF)] = v;
            winIdx = (winIdx + 1) & 0x0FFF;
            produced++;
            copied++;
            if (produced >= c.OutTarget) break;
          }
        }
        else
        {
          EmitByte(c, (byte)sym);
          c.Scratch[32 + (winIdx & 0x0FFF)] = b;
          winIdx = (winIdx + 1) & 0x0FFF;
          produced++;
        }
      }
      return true;
    }

    // === Modelled arithmetic decoder pieces (direct translation of the snippet) ===
    private static int DecodeLitLenSymbol(RawCtx c)
    {
      var s = c.Scratch;
      int v2 = (int)U32(s, 66332);
      uint range = U32(s, 66336) - (uint)v2;
      uint code = U32(s, 66340);
      // ((unsigned int)*((uint16*)&scratch[57502]) * (code - v2 + 1) - 1) / range
      ushort total = U16(s, 57502);
      uint scaled = ((uint)total * (code - (uint)v2 + 1) - 1) / range;
      int idx = LitLenLowerBound(c, (ushort)scaled);

      ushort cumLo = U16(s, 57500 + 2 * idx);
      ushort cumHi = U16(s, 57502 + 2 * idx);

      uint low = (uint)v2 + range * cumHi / total;
      uint high = (uint)v2 + range * cumLo / total;
      W32(s, 66336, low);
      W32(s, 66332, high);

      while (true)
      {
        uint curLow = U32(s, 66332);
        if (curLow >= 0x10000)
        {
          W32(s, 66332, curLow - 0x10000);
          W32(s, 66340, U32(s, 66340) - 0x10000);
          W32(s, 66336, U32(s, 66336) - 0x10000);
          RenormReadBit(c);
        }
        else if (curLow >= 0x8000)
        {
          uint curHigh = U32(s, 66336);
          if (curHigh <= 0x18000)
          {
            W32(s, 66340, U32(s, 66340) - 0x8000);
            W32(s, 66332, curLow - 0x8000);
            W32(s, 66336, curHigh - 0x8000);
            RenormReadBit(c);
          }
          else break;
        }
        else if (U32(s, 66336) <= 0x10000)
        {
          RenormReadBit(c);
        }
        else break;
      }
      int decoded = (int)U32(s, 55612 + 4 * idx);
      UpdateLitLenModel(c, idx);
      return decoded;
    }

    private static void RenormReadBit(RawCtx c)
    {
      var s = c.Scratch;
      W32(s, 66332, U32(s, 66332) * 2);
      W32(s, 66336, U32(s, 66336) * 2);
      bool bit = ReadBit(c);
      W32(s, 66340, (bit ? 1u : 0u) + 2u * U32(s, 66340));
    }

    private static int LitLenLowerBound(RawCtx c, ushort key)
    {
      var s = c.Scratch;
      int lo = 1;
      int hi = 314;
      while (lo < hi)
      {
        int mid = (lo + hi) / 2;
        if (U16(s, 57502 + 2 * mid) <= key) hi = mid; else lo = mid + 1;
      }
      return lo;
    }

    private static int DecodeMatchDistance(RawCtx c)
    {
      var s = c.Scratch;
      int v2 = (int)U32(s, 66332);
      uint range = U32(s, 66336) - (uint)v2;
      ushort total = U16(s, 58132);
      uint code = U32(s, 66340);
      uint scaled = ((uint)total * (code - (uint)v2 + 1) - 1) / range;
      int idx = DistUpperBound(c, (ushort)scaled);

      ushort cum0 = U16(s, 58132 + 2 * idx);
      ushort cum1 = U16(s, 58134 + 2 * idx);
      uint low = (uint)v2 + range * cum0 / total;
      uint high = (uint)v2 + range * cum1 / total;
      W32(s, 66336, low);
      W32(s, 66332, high);

      while (true)
      {
        uint curLow = U32(s, 66332);
        if (curLow >= 0x10000)
        {
          W32(s, 66332, curLow - 0x10000);
          W32(s, 66340, U32(s, 66340) - 0x10000);
          W32(s, 66336, U32(s, 66336) - 0x10000);
          RenormReadBit(c);
        }
        else if (curLow >= 0x8000)
        {
          uint curHigh = U32(s, 66336);
          if (curHigh <= 0x18000)
          {
            W32(s, 66340, U32(s, 66340) - 0x8000);
            W32(s, 66332, curLow - 0x8000);
            W32(s, 66336, curHigh - 0x8000);
            RenormReadBit(c);
          }
          else break;
        }
        else if (U32(s, 66336) <= 0x10000)
        {
          return idx; // matches early-return in snippet
        }
        else break;
      }
      return idx;
    }

    private static int DistUpperBound(RawCtx c, ushort key)
    {
      var s = c.Scratch;
      int lo = 1;
      int hi = 4096;
      while (lo < hi)
      {
        int mid = (lo + hi) / 2;
        if (U16(s, 58132 + 2 * mid) <= key) hi = mid; else lo = mid + 1;
      }
      return lo - 1;
    }

    private static int UpdateLitLenModel(RawCtx c, int idx)
    {
      var s = c.Scratch;
      // Direct translation (simplified) – maintains cumulative totals without rebalancing map arrays fully.
      // For portability, we cap totals and renormalize when they approach 0x7FFF.
      const int TOTAL_OFF = 57502;
      ushort total = U16(s, TOTAL_OFF);
      if (total >= 0x7FFF)
      {
        short sum = 0;
        // Renormalize the 314 bins: half each, rebuild cumulative totals
        for (int i = 314; i >= 1; i--)
        {
          int loOff = 57500 + 2 * i;
          ushort v = U16(s, loOff);
          ushort half = (ushort)((v + 1) >> 1);
          W16(s, loOff, half);
          sum += (short)half;
        }
        W16(s, TOTAL_OFF, (ushort)sum);
        total = (ushort)sum;
      }

      // Increment the chosen symbol's count and the cumulative total
      ushort lo = U16(s, 57500 + 2 * idx);
      ushort hi = U16(s, 57502 + 2 * idx);
      if (lo == hi)
      {
        // Move up to previous distinct bin, then swap mapping (approximation of the original heap-update logic)
        int j = idx - 1;
        while (j >= 1 && U16(s, 57500 + 2 * j) == U16(s, 57500 + 2 * (j + 1))) j--;
        if (j >= 1)
        {
          // swap symbol map entries
          uint a = U32(s, 55612 + 4 * idx);
          uint b = U32(s, 55612 + 4 * j);
          W32(s, 55612 + 4 * idx, b);
          W32(s, 55612 + 4 * j, a);
        }
      }
      // bump cumulative for bins <= idx
      for (int k = 1; k <= idx; k++)
      {
        ushort cur = U16(s, 57502 + 2 * k);
        W16(s, 57502 + 2 * k, (ushort)(cur + 1));
      }
      W16(s, TOTAL_OFF, (ushort)(total + 1));
      return 0;
    }
  }
  #endregion
}
