#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>

#define MAX_DOCS 10
#define MAX_WORDS 100

typedef struct
{
    char word[50];

    // frequency of word in each document
    int term_frequency[MAX_DOCS];

    // normalized term frequency
    double tf[MAX_DOCS];

    // TF-IDF value for each document
    double TF_IDF[MAX_DOCS];

    // inverse document frequency
    double idf;

    // number of documents containing the word
    int document_frequency;

} WordData;

typedef struct
{
    char sentence[1000]; // raw document text
    int total_words;     // total words in document
    WordData words[MAX_WORDS];
    int word_count; // number of unique words

} FILES;

/* ---------- FUNCTION PROTOTYPES ---------- */

void read_file_words(const char *filename, int i, FILES file[]);
int is_stopword(const char *word);
int find_word(const char *word, int doc_index, FILES file[]);
void document_freq(FILES file[], int n);
void add_or_update_word(const char *word, int i, FILES file[]);
double calculate_tf(int word_count, int total_words);
double calculate_idf(int document_frequency, int n);
double calculate_cosine_similarity(FILES file[], int A, int B);

/* ---------- MAIN ---------- */

int main(int argc, char *argv[])
{

    // minimum 2 files required
    if (argc <= 2)
    {
        printf("not enough textfiles entered for %s to function, run through the terminal\n", argv[0]);
        return 1;
    }

    int n = argc - 1;

    // dynamically allocate memory for documents
    FILES *file = calloc(n, sizeof(FILES));

    // checks if memory allocation succeeded
    if (file == NULL)
    {
        printf("Memory allocation failed.\n");
        return 1;
    }

    /* ---------- READ FILES ---------- */

    for (int i = 0; i < n; i++)
    {
        read_file_words(argv[i + 1], i, file);
    }

    /* ---------- DOCUMENT FREQUENCY ---------- */

    document_freq(file, n);

    /* ---------- TF / IDF / TF-IDF ---------- */

    for (int j = 0; j < n; j++)
    {
        for (int k = 0; k < file[j].word_count; k++)
        {
            double idf =
                calculate_idf(
                    file[j].words[k].document_frequency,
                    n);

            file[j].words[k].idf = idf;
            // calculates tf and tf-idf
            for (int d = 0; d < n; d++)
            {
                int tfreq =
                    file[j].words[k].term_frequency[d];

                if (tfreq > 0)
                { // calculate tf if term frequency > 0
                    file[j].words[k].tf[d] =
                        calculate_tf(
                            tfreq,
                            file[d].total_words);

                    file[j].words[k].TF_IDF[d] =
                        file[j].words[k].tf[d] * idf;
                }
                else
                {   //if the word is not found anywhere, to avoid unallocated array values to use memory effectively
                    file[j].words[k].tf[d] = 0.0;
                    file[j].words[k].TF_IDF[d] = 0.0;
                }
            }
        }
    }

    /* ----------TF-IDF OUTPUT OF EACH WORD ---------- */

    for (int j = 0; j < n; j++)
    {
        printf("\nDocument %d TF-IDF values:\n", j + 1);

        for (int k = 0; k < file[j].word_count; k++)
        {
            printf(
                "Word: %-15s TF: %.4f IDF: %.4f TF-IDF: %.4f\n",

                file[j].words[k].word,
                file[j].words[k].tf[j],
                file[j].words[k].idf,
                file[j].words[k].TF_IDF[j]);
        }
    }

    /* ---------- SIMILARITY REPORT ---------- */

    printf("\nTF-IDF Text Similarity Report----------------------------\n");

    for (int i = 0; i < n; i++)
    {
        printf("File %d: %s\n", i + 1, argv[i + 1]);
    }

    /* ---------- COMPARE ALL DOCUMENTS ---------- */

    for (int i = 0; i < n; i++)
    {
        for (int j = i + 1; j < n; j++)
        {
            double sim =
                calculate_cosine_similarity(file, i, j);

            printf(
                "\nSimilarity between File %d and File %d: %.2f\n",
                i + 1,
                j + 1,
                sim);

            if (sim > 0.8)
            {
                printf("Interpretation: Highly similar texts.\n");
            }
            else if (sim > 0.5)
            {
                printf("Interpretation: Moderately similar texts.\n");
            }
            else
            {
                printf("Interpretation: Quite different texts.\n");
            }
        }
    }

    // releases dynamically allocated memory
    free(file);
    return 0;
}

/* ---------- READ FILE ---------- */

void read_file_words(const char *filename, int i, FILES file[])
{
    FILE *f = fopen(filename, "r");

    if (f == NULL)
    {
        printf("File not found.\n");
        exit(EXIT_FAILURE);
    }

    file[i].sentence[0] = '\0';

    char sentence[200];

    // read entire file
    while (fgets(sentence, sizeof(sentence), f) != NULL)
    {
        strcat(file[i].sentence, sentence);
    }

    int total_count = 0;

    char word[50];
    word[0] = '\0';//initiallise the word string

    /* ---------- SPLIT TEXT INTO WORDS ---------- */

    for (int j = 0;
         file[i].sentence[j] != '\0';
         j++)
    {
        if (file[i].sentence[j] != ' ' &&
            file[i].sentence[j] != '\n')
        {
            int len = strlen(word);

            // prevent word buffer overflow
            if (len < 49)
            {
                word[len] = file[i].sentence[j];
                word[len + 1] = '\0';
            }
        }
        else
        {
            if (word[0] != '\0')
            {
                int len = strlen(word);

                // remove punctuation
                if (len > 0)
                {
                    char last = word[len - 1];

                    if (last == '.' ||
                        last == ',' ||
                        last == '!' ||
                        last == '?')
                    {
                        word[len - 1] = '\0';
                    }
                }

                // lowercase conversion
                for (int k = 0; k < strlen(word); k++)
                {
                    word[k] = tolower(word[k]);
                }

                total_count++;

                // ignore stopwords
                if (!is_stopword(word))
                {
                    add_or_update_word(word, i, file);
                }

                // reset word buffer
                word[0] = '\0';
            }
        }
    }

    /* ---------- HANDLE LAST WORD ---------- */

    if (word[0] != '\0')
    {
        int len = strlen(word);

        if (len > 0)
        {
            char last = word[len - 1];

            if (last == '.' ||
                last == ',' ||
                last == '!' ||
                last == '?')
            {
                word[len - 1] = '\0';
            }
        }

        // lowercase conversion
        for (int k = 0; k < strlen(word); k++)
        {
            word[k] = tolower(word[k]);
        }

        total_count++;

        // ignore common words with little meaning
        if (!is_stopword(word))
        {
            add_or_update_word(word, i, file);
        }
    }

    file[i].total_words = total_count;

    fclose(f);

    /* ---------- DOCUMENT SUMMARY ---------- */

    printf("\nDocument %d summary:\n", i + 1);

    printf("Sentence: %s\n", file[i].sentence);

    printf("Total words: %d\n",
           file[i].total_words);

    printf("Unique words: %d\n",
           file[i].word_count);

    printf("Word frequencies:\n");

    for (int j = 0; j < file[i].word_count; j++)
    {
        printf("%s : %d\n",

               file[i].words[j].word,
               file[i].words[j].term_frequency[i]);
    }
}

/* ---------- STOPWORD CHECK ---------- */

int is_stopword(const char *word)
{
    const char *stopwords[] =
        {
            "the", "is", "and", "of", "a", "in", "to", "with", "for", "on",
            "at", "by", "an", "be", "this", "that", "it", "as", "are",
            "was", "were", "from", "or", "but", "if", "then", "than"};

    int stopword_count =27;

    for (int i = 0; i < stopword_count; i++)
    {
        if (strcmp(word, stopwords[i]) == 0)
        {
            return 1;
        }
    }

    return 0;
}

/* ---------- ADD OR UPDATE WORD ---------- */

void add_or_update_word(const char *word,
                        int i,
                        FILES file[])
{
    int pos = find_word(word, i, file);

    // existing word
    if (pos != -1)
    {
        file[i].words[pos].term_frequency[i]++;
    }
    else
    {
        // new word
        strcpy(
            file[i].words[file[i].word_count].word,
            word);

        file[i].words[file[i].word_count].term_frequency[i] = 1;

        file[i].word_count++;
    }
}

/* ---------- FIND WORD ---------- */

int find_word(const char *word,
              int doc_index,
              FILES file[])
{
    for (int u = 0;
         u < file[doc_index].word_count;
         u++)
    {
        if (strcmp(
                file[doc_index].words[u].word,
                word) == 0)
        {
            return u;
        }
    }

    return -1;
}

/* ---------- DOCUMENT FREQUENCY ---------- */

void document_freq(FILES file[], int n)
{
    for (int i = 0; i < n; i++)
    {
        printf("\nDocument %d:\n", i + 1);

        for (int j = 0;
             j < file[i].word_count;
             j++)
        {
            file[i].words[j].document_frequency = 0;

            for (int l = 0; l < n; l++)
            {
                for (int k = 0;
                     k < file[l].word_count;
                     k++)
                {
                    if (strcmp(
                            file[i].words[j].word,
                            file[l].words[k].word) == 0)
                    {
                        file[i].words[j].document_frequency++;

                        break;
                    }
                }
            }

            printf(
                "Word '%s' appears in %d document(s)\n",

                file[i].words[j].word,

                file[i].words[j].document_frequency);
        }
    }
}

/* ---------- TF ---------- */

double calculate_tf(int word_count,
                    int total_words)
{
    return (double)word_count /
           (double)total_words;
}

/* ---------- IDF ---------- */

double calculate_idf(int document_frequency,
                     int n)
{
    // smoothed IDF
    return log(
               (double)(n + 1) /
               (double)(document_frequency + 1)) +
           1;
}

/* ---------- COSINE SIMILARITY ---------- */

double calculate_cosine_similarity(FILES file[],
                                   int A,
                                   int B)
{
    double dot = 0.0;

    double magA = 0.0;
    double magB = 0.0;

    /* ---------- MAGNITUDE A ---------- */

    for (int i = 0;
         i < file[A].word_count;
         i++)
    {
        double valA =
            file[A].words[i].TF_IDF[A];

        magA += valA * valA;
    }

    /* ---------- MAGNITUDE B ---------- */

    for (int i = 0;
         i < file[B].word_count;
         i++)
    {
        double valB =
            file[B].words[i].TF_IDF[B];

        magB += valB * valB;
    }

    /* ---------- DOT PRODUCT ---------- */

    for (int i = 0;
         i < file[A].word_count;
         i++)
    {
        char *wordA =
            file[A].words[i].word;

        double valA =
            file[A].words[i].TF_IDF[A];

        double valB = 0.0;

        // search for matching word
        for (int j = 0;
             j < file[B].word_count;
             j++)
        {
            if (strcmp(
                    wordA,
                    file[B].words[j].word) == 0)
            {
                valB =
                    file[B].words[j].TF_IDF[B];

                break;
            }
        }

        dot += valA * valB;
    }

    /* ---------- ZERO VECTOR CHECK ---------- */

    if (magA == 0 && magB == 0)
    {
        if (strcmp(
                file[A].sentence,
                file[B].sentence) == 0)
        {
            return 1.0;
        }
        else
        {
            return 0.0;
        }
    }

    if (magA == 0 || magB == 0)
    {
        return 0.0;
    }

    /* ---------- FINAL COSINE SIIMILARITY FORMULA ---------- */

    return dot /
           (sqrt(magA) * sqrt(magB));
}