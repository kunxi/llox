import { defineCollection, z } from "astro:content";

const chapters = defineCollection({
  schema: z.object({
    title: z.string(),
    kind: z.enum(["part", "chapter", "backmatter"]),
    part: z.string().optional(),      // "I", "II", "III" (chapters only)
    number: z.number().optional(),    // chapter or appendix number
    partNumber: z.string().optional(), // "I", "II", "III" (part intros only)
    order: z.number(),                // global ordering
  }),
});

export const collections = { chapters };
