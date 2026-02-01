using System;
using System.Collections.Generic;
using System.IO;
using System.Text;

using QEngine;
using QEngine.GUI;
using QEngine.Text;

using SixLabors.ImageSharp;
using SixLabors.ImageSharp.PixelFormats;

using Veldrid;
using Veldrid.SPIRV;

using Image = SixLabors.ImageSharp.Image;

struct Vertex
{
    public Vector2 Position;
    public Vector4 Color;

    public Vertex(Vector2 pos, Vector4 col)
    {
        Position = pos;
        Color = col;
    }

    public static uint SizeInBytes => (2 + 4) * 8;
}
struct SpriteVertex
{
    public Vector2 Position;
    public Vector2 UV;
    public Vector4 Color;
    
    public SpriteVertex(Vector2 p, Vector2 uv, Vector4 c)
    {
        Position = p;
        UV = uv;
        Color = c;
    }

    public static uint SizeInBytes => (2 + 2 + 4) * 4;
}

namespace QEngine.Dev.Renderer
{
    public static class Atlas
    {
    public static Texture atlasTexture;
    public static TextureView atlasView;
    
        static byte[] pixels;
        static Vector2Int size = new();
        static Vector2Int cursor = new();
        static int rowHeight = 0;

        static List<(Sprite sprite, Vector2Int pos)> sprites = new();
        static int maxSlotX = 8;
        static int slotX = 0;

        public static void AddFont(Sprite sprite)
        {
            Console.WriteLine("Adding font to Atlas");
            sprites.Add((sprite, cursor));
            cursor.y += sprite.Height;
            if (cursor.y + rowHeight > size.y) size.y = cursor.y + rowHeight;
            if (sprite.Width > size.x) size.x = sprite.Width;
        }
        public static void AddImage(Sprite sprite)
        {
            Console.WriteLine("Adding Sprite to Atlas");
            if (slotX >= maxSlotX)
            {
                cursor.x = 0;
                cursor.y += rowHeight;
                rowHeight = 0;
                slotX = 0;
            }
            sprites.Add((sprite, cursor));

            cursor.x += sprite.Width;
            if (sprite.Height > rowHeight)
                rowHeight = sprite.Height;
            slotX++;

            if (cursor.x > size.x) size.x = cursor.x;
            if (cursor.y + rowHeight > size.y) size.y = cursor.y + rowHeight;
        }

        public static void Init(out Vector2Int s)
        {
            pixels = new byte[size.x * size.y * 4];
            int bpp = 4;
            foreach (var entry in sprites)
            {
                Sprite sprite = entry.sprite;
                Vector2Int pos = entry.pos;
                for (int y = 0; y < sprite.Height; y++)
                {
                    int atlasStart = ((pos.y + y) * size.x + pos.x) * bpp;
                    int spriteStart = y * sprite.Width * bpp;
                    Array.Copy(sprite.pixels, spriteStart,
                        pixels, atlasStart, sprite.Width * bpp);
                }
                sprite.uv = new Vector4(
                    pos.x / (float)size.x,
                    pos.y / (float)size.y,
                    (pos.x + sprite.Width) / (float)size.x,
                    (pos.y + sprite.Height) / (float)size.y
                );
                Console.WriteLine($"Calculate sprite in Atlas: {sprite.uv}");
            }
            s = size;
            Save();
        }
        
        public static void CreateAtlasTexture(GraphicsDevice gd)
        {
            TextureDescription desc = TextureDescription.Texture2D(
                (uint)size.x, (uint)size.y, 1, 1,
                PixelFormat.R8_G8_B8_A8_UNorm, 
                TextureUsage.Sampled);

            atlasTexture = gd.ResourceFactory.CreateTexture(desc);

            // upload pixels
            gd.UpdateTexture(atlasTexture, pixels, 0, 0, 0, (uint)size.x, (uint)size.y, 1, 0, 0);

            atlasView = gd.ResourceFactory.CreateTextureView(atlasTexture);
        }
        static void Save(string fileName = "Atlas.png")
        {
            Directory.CreateDirectory(Path.Combine(Directory.GetCurrentDirectory(), "Assets"));
            using var img = Image.LoadPixelData<Rgba32>(pixels, size.x, size.y);
            img.Save(Path.Combine(Directory.GetCurrentDirectory(), "Assets", fileName));
        }
    }
    // ================= DATA =================
    public struct BatchedShape
    {
        public Vector2 pixelCenter;
        public Vector2[] localPositions;
        public ushort[] indices;
        public Vector4 color;
    }
    public struct BatchedSprite
    {
        public Sprite sprite;
        public Vector4 uv;
        public Vector2 position;
        public Vector2 size;
        public Vector4 color;
    }

    // ================= RENDERER =================
    public static class QRenderer
    {
        static GraphicsDevice gd;
        static CommandList cl;

        static Pipeline shapePipeline;
        static Pipeline spritePipeline;

        static DeviceBuffer shapeVB;
        static DeviceBuffer shapeIB;

        static DeviceBuffer spriteVB;
        static DeviceBuffer spriteIB;

        static Sampler sampler;

        static List<BatchedShape> shapesThisFrame = new();
        static List<BatchedSprite> spritesThisFrame = new();

        // ================= INIT =================
        public static void Init(GraphicsDevice device)
        {
            gd = device;
            cl = gd.ResourceFactory.CreateCommandList();

            shapeVB = gd.ResourceFactory.CreateBuffer(
                new BufferDescription(8192 * Vertex.SizeInBytes, BufferUsage.VertexBuffer | BufferUsage.Dynamic));
            shapeIB = gd.ResourceFactory.CreateBuffer(
                new BufferDescription(8192 * sizeof(ushort), BufferUsage.IndexBuffer | BufferUsage.Dynamic));

            spriteVB = gd.ResourceFactory.CreateBuffer(
                new BufferDescription(8192 * SpriteVertex.SizeInBytes, BufferUsage.VertexBuffer | BufferUsage.Dynamic));
            spriteIB = gd.ResourceFactory.CreateBuffer(
                new BufferDescription(8192 * sizeof(ushort), BufferUsage.IndexBuffer | BufferUsage.Dynamic));

            sampler = gd.ResourceFactory.CreateSampler(new SamplerDescription(
                SamplerAddressMode.Clamp,
                SamplerAddressMode.Clamp,
                SamplerAddressMode.Clamp,
                SamplerFilter.MinPoint_MagPoint_MipPoint,
                ComparisonKind.Never,
                0, 0, 0, 0,
                SamplerBorderColor.OpaqueBlack));

            atlasLayout = gd.ResourceFactory.CreateResourceLayout(
                new ResourceLayoutDescription(
                    new ResourceLayoutElementDescription("mySampler", ResourceKind.Sampler, ShaderStages.Fragment),
                    new ResourceLayoutElementDescription("myTexture", ResourceKind.TextureReadOnly, ShaderStages.Fragment)
                ));

            // -------- SHAPE PIPELINE --------
            var shapeShaders = gd.ResourceFactory.CreateFromSpirv(
                new ShaderDescription(ShaderStages.Vertex, Encoding.UTF8.GetBytes(ShapeVS), "main"),
                new ShaderDescription(ShaderStages.Fragment, Encoding.UTF8.GetBytes(ShapeFS), "main"));

            shapePipeline = gd.ResourceFactory.CreateGraphicsPipeline(
                new GraphicsPipelineDescription
                {
                    BlendState = BlendStateDescription.SingleAlphaBlend,
                    DepthStencilState = DepthStencilStateDescription.Disabled,
                    RasterizerState = RasterizerStateDescription.CullNone,
                    PrimitiveTopology = PrimitiveTopology.TriangleList,
                    ShaderSet = new ShaderSetDescription(
                        new[]
                        {
                            new VertexLayoutDescription(
                                new VertexElementDescription("Position", VertexElementSemantic.Position, VertexElementFormat.Float2),
                                new VertexElementDescription("Color", VertexElementSemantic.Color, VertexElementFormat.Float4))
                        },
                        shapeShaders),
                    ResourceLayouts = Array.Empty<ResourceLayout>(),
                    Outputs = gd.SwapchainFramebuffer.OutputDescription
                });

            // -------- SPRITE PIPELINE --------

            var spriteShaders = gd.ResourceFactory.CreateFromSpirv(
                new ShaderDescription(ShaderStages.Vertex, Encoding.UTF8.GetBytes(SpriteVS), "main"),
                new ShaderDescription(ShaderStages.Fragment, Encoding.UTF8.GetBytes(SpriteFS), "main"));

            spritePipeline = gd.ResourceFactory.CreateGraphicsPipeline(
                new GraphicsPipelineDescription
                {
                    BlendState = BlendStateDescription.SingleAlphaBlend,
                    DepthStencilState = DepthStencilStateDescription.Disabled,
                    RasterizerState = RasterizerStateDescription.CullNone,
                    PrimitiveTopology = PrimitiveTopology.TriangleList,
                    ShaderSet = new ShaderSetDescription(
                        new[]
                        {
                            new VertexLayoutDescription(
                                new VertexElementDescription("Position", VertexElementSemantic.Position, VertexElementFormat.Float2),
                                new VertexElementDescription("UV", VertexElementSemantic.TextureCoordinate, VertexElementFormat.Float2),
                                new VertexElementDescription("Color", VertexElementSemantic.Color, VertexElementFormat.Float4))
                        },
                        spriteShaders),
                    ResourceLayouts = new[] { atlasLayout },
                    Outputs = gd.SwapchainFramebuffer.OutputDescription
                });
        }

        public static void Begin()
        {
            cl = gd.ResourceFactory.CreateCommandList();
            cl.Begin();
            cl.SetFramebuffer(gd.SwapchainFramebuffer);
            cl.ClearColorTarget(0, Game.background.toRgba());

            shapesThisFrame.Clear();
            spritesThisFrame.Clear();
        }

        public static void End()
        {
            FlushShapes();
            FlushSprites();

            cl.End();
            gd.SubmitCommands(cl);
            gd.SwapBuffers();
            cl.Dispose();
        }
        public static void CreateAtlasSampler(GraphicsDevice gd)
        {
            sampler = gd.ResourceFactory.CreateSampler(new SamplerDescription(
                SamplerAddressMode.Clamp, SamplerAddressMode.Clamp, SamplerAddressMode.Clamp,
                SamplerFilter.MinPoint_MagPoint_MipPoint, ComparisonKind.Never,
                0, 0, 0, 0,
                SamplerBorderColor.OpaqueBlack));
        }
        static ResourceLayout atlasLayout;
        static ResourceSet atlasSet;
        public static void CreateAtlasResourceSet(GraphicsDevice gd)
        => atlasSet = gd.ResourceFactory.CreateResourceSet(new ResourceSetDescription(atlasLayout, 
            sampler, Atlas.atlasView));
        // ================= API =================
        public static void DrawShape(Vector2 center, Vector2[] verts, ushort[] inds, Vector4 color)
        {
            shapesThisFrame.Add(new BatchedShape
            {
                pixelCenter = center,
                localPositions = verts,
                indices = inds,
                color = color
            });
        }

        public static void DrawSprite(Sprite sprite, Vector4 uv, Vector2 pos, Vector2 size, Vector4 color)
        {
            spritesThisFrame.Add(new BatchedSprite
            {
                sprite = sprite,
                uv = uv,
                position = pos,
                size = size,
                color = color
            });
        }
        
        public static void DrawText(Font font, float space, string text, Vector2 position, int fontSize, Vector4 color)
        {
            float scale = fontSize / font.defaultFontSize;
            Vector2 cursor = new();
            foreach (char c in text)
            {
                if (c == '\n')
                {
                    cursor.x = 0;
                    cursor.y -= font.charSize.y * scale;
                    continue;
                }

                if (c == ' ')
                {
                    cursor.x += font.charSize.x / 2f * scale;
                    continue;
                }
                if (!font.glyphs.TryGetValue(c, out Glyph g))
                    continue;
                Vector2 size = new Vector2(font.charSize.x, font.charSize.y) * scale;
                DrawSprite(
                    font.texture,
                    new Vector4(g.uvMin.x, g.uvMin.y, g.uvMax.x, g.uvMax.y),
                    position + cursor + new Vector2(g.thick * scale, size.y) / 2,
                    size, color
                );
                cursor.x += space + g.thick * scale;
            }
        }
        static DeviceBuffer EnsureBuffer(
            DeviceBuffer buffer,
            uint requiredBytes,
            BufferUsage usage)
        {
            if (buffer == null || buffer.SizeInBytes < requiredBytes)
            {
                buffer?.Dispose();

                uint newSize = Math.Max(requiredBytes, requiredBytes * 2);

                buffer = gd.ResourceFactory.CreateBuffer(
                    new BufferDescription(newSize, usage));
            }
            return buffer;
        }
        // ================= FLUSH =================
        static void FlushShapes()
        {
            if (shapesThisFrame.Count == 0)
                return;
            int totalVerts = 0;
            int totalIndices = 0;

            foreach (var s in shapesThisFrame)
            {
                totalVerts += s.localPositions.Length;
                totalIndices += s.indices.Length;
            }

            uint requiredVB =
                (uint)(totalVerts * Vertex.SizeInBytes);

            uint requiredIB =
                (uint)(totalIndices * sizeof(ushort));

            shapeVB = EnsureBuffer(
                shapeVB, requiredVB,
                BufferUsage.VertexBuffer | BufferUsage.Dynamic);

            shapeIB = EnsureBuffer(
                shapeIB, requiredIB,
                BufferUsage.IndexBuffer | BufferUsage.Dynamic);
            
            List<Vertex> v = new(totalVerts);
            List<ushort> i = new(totalIndices);
            
            foreach (var s in shapesThisFrame)
            {
                ushort baseIndex = (ushort)v.Count;

                foreach (var p in s.localPositions)
                    v.Add(new Vertex(Camera.PixelToNDC(s.pixelCenter + p), s.color));

                foreach (var idx in s.indices)
                    i.Add((ushort)(idx + baseIndex));
            }

            gd.UpdateBuffer(shapeVB, 0, v.ToArray());
            gd.UpdateBuffer(shapeIB, 0, i.ToArray());

            cl.SetPipeline(shapePipeline);
            cl.SetVertexBuffer(0, shapeVB);
            cl.SetIndexBuffer(shapeIB, IndexFormat.UInt16);
            cl.DrawIndexed((uint)i.Count, 1, 0, 0, 0);
        }

        static void FlushSprites()
        {
            int spriteCount = spritesThisFrame.Count;
            if (spriteCount == 0) return;

            uint requiredVB =
                (uint)(spriteCount * 4 * SpriteVertex.SizeInBytes);

            uint requiredIB =
                (uint)(spriteCount * 6 * sizeof(ushort));

            spriteVB = EnsureBuffer(
                spriteVB, requiredVB,
                BufferUsage.VertexBuffer | BufferUsage.Dynamic);

            spriteIB = EnsureBuffer(
                spriteIB, requiredIB,
                BufferUsage.IndexBuffer | BufferUsage.Dynamic);

            SpriteVertex[] verts = new SpriteVertex[spriteCount * 4];
            ushort[] indices = new ushort[spriteCount * 6];

            int vo = 0, io = 0;
            foreach (var s in spritesThisFrame)
                WriteQuad(verts, indices, ref vo, ref io, s);

            gd.UpdateBuffer(spriteVB, 0, verts);
            gd.UpdateBuffer(spriteIB, 0, indices);

            cl.SetPipeline(spritePipeline);
            cl.SetVertexBuffer(0, spriteVB);
            cl.SetIndexBuffer(spriteIB, IndexFormat.UInt16);
            cl.SetGraphicsResourceSet(0, atlasSet);
            cl.DrawIndexed((uint)indices.Length, 1, 0, 0, 0);

            spritesThisFrame.Clear();
        }

        static void WriteQuad(SpriteVertex[] v, ushort[] i, ref int vo, ref int io, BatchedSprite s)
        {
            Vector2 c = Camera.PixelToNDC(s.position);
            Vector2 h = Camera.SizeToNDC(s.size) * 0.5f;

            v[vo + 0] = new(c + new Vector2(-h.x, -h.y), new(s.uv.x, s.uv.y), s.color);
            v[vo + 1] = new(c + new Vector2(+h.x, -h.y), new(s.uv.z, s.uv.y), s.color);
            v[vo + 2] = new(c + new Vector2(+h.x, +h.y), new(s.uv.z, s.uv.w), s.color);
            v[vo + 3] = new(c + new Vector2(-h.x, +h.y), new(s.uv.x, s.uv.w), s.color);


            i[io + 0] = (ushort)(vo + 0);
            i[io + 1] = (ushort)(vo + 1);
            i[io + 2] = (ushort)(vo + 2);
            i[io + 3] = (ushort)(vo + 2);
            i[io + 4] = (ushort)(vo + 3);
            i[io + 5] = (ushort)(vo + 0);

            vo += 4;
            io += 6;
        }

        // ================= SHADERS =================
        const string ShapeVS = @"#version 450
layout(location=0) in vec2 Position;
layout(location=1) in vec4 Color;
layout(location=0) out vec4 fsColor;
void main(){ gl_Position=vec4(Position,0,1); fsColor=Color; }";

        const string ShapeFS = @"#version 450
layout(location=0) in vec4 fsColor;
layout(location=0) out vec4 outColor;
void main(){ outColor=fsColor; }";

        const string SpriteVS = @"#version 450
layout(location=0) in vec2 Position;
layout(location=1) in vec2 UV;
layout(location=2) in vec4 Color;
layout(location=0) out vec2 fsUV;
layout(location=1) out vec4 fsColor;
void main(){ gl_Position=vec4(Position,0,1); fsUV=UV; fsColor=Color; }";

        const string SpriteFS = @"#version 450
layout(location=0) in vec2 fsUV;
layout(location=1) in vec4 fsColor;
layout(set=0,binding=0) uniform sampler mySampler;
layout(set=0,binding=1) uniform texture2D myTexture;
layout(location=0) out vec4 outColor;
void main(){ outColor=texture(sampler2D(myTexture,mySampler),fsUV)*fsColor; }";
    }
}
