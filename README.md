# TF‑IDF Text Similarity

A C program that computes TF‑IDF (Term Frequency–Inverse Document Frequency) vectors for multiple text documents and calculates the cosine similarity between every pair.

## Features

- Reads any number of text files (up to 10) from the command line.
- Removes common stopwords (e.g., "the", "is", "and").
- Converts words to lowercase and strips trailing punctuation.
- Computes:
  - Term frequency (TF) per document.
  - Inverse document frequency (IDF) with smoothing.
  - TF‑IDF values for each word in each document.
- Outputs a similarity matrix with human‑readable interpretations:
  - **> 0.8** → Highly similar
  - **0.5 – 0.8** → Moderately similar
  - **< 0.5** → Quite different

## Compilation

```bash
gcc -o tfidf_similarity tfidf_similarity.c -lm
