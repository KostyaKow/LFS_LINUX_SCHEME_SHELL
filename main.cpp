#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <iostream>

using namespace std;

//for non standard-confronting compilers
#define MINIMIZE_STD_FUNCS 0

#if !MINIMIZE_STD_FUNCS
   #include <assert.h>
   #include <stdint.h>
   #include <stdbool.h>
   #include <string.h> //strlen
#endif

typedef uint8_t* pointer;

/***** ARRAY SUPER INEFFICIENT CAUSE POINTERS, AND I DON'T WANT TO USE PREPROCESSOR MAGIC *****/

typedef struct array {
   uint8_t** arr;
   size_t num, len, *size_each;
} array_t;

void array_new(array_t* arr) {
   arr->arr = malloc(sizeof(uint8_t*)*5);
   arr->size_each = malloc(sizeof(size_t)*5);
   arr->num = 0;
   arr->len = 5;
}

//makes a copy
void array_add(array_t* arr, void* item, size_t size) {
   if (arr->num + 1 >= arr->len) {
      arr->len *= 2;
      arr->size_each = realloc(arr->size_each, arr->len * sizeof(size_t));
      arr->arr = realloc(arr->arr, arr->len * sizeof(pointer));
   }
   arr->arr[arr->num] = malloc(size);
   arr->size_each[arr->num] = size;
   memcpy(arr->arr[arr->num++], item, size);
}

//only adds pointer
void array_add_ptr(array_t* arr, pointer item) {}

//#define array_get(arr_, index, type) (*((type*)(&((arr_)->arr[index]))))

#define array_get(arr_, index, type) \
  (*((type*)((arr_)->arr[index])))

#define array_for_each(arr_, i_name) \
   for (int i_name = 0; i_name < (arr_).num; i_name++)

void _print_array_info(array_t* arr) {
   printf("\nnum: %zu, len: %zu",
          arr->num, arr->len);
}

bool arr_test() {
   char* s = "Hello, World!";

   array_t arr;
   array_new(&arr);
   _print_array_info(&arr);

   for (int i = 0; i < strlen(s); i++) {
      array_add(&arr, &(s[i]), 1);
      _print_array_info(&arr);
      //printf("---elem: %c", array_get(&arr, i, char));
      printf("---elem: %c", array_get(&arr, i, char));
   }
   //array_delete(&arr, &default_elem_free, false);
   return true;
}

enum { PARSE_UNTERM_STR };

#define scm_error(x) assert(0);
/*void scm_error(int i) {
   assert(0);
}*/

array_t* lex(const char* s) {
   int s_len = strlen(s);
   array_t* arr = malloc(sizeof(array_t));
   array_new(arr);

   char* curr_lex;
   int curr_lex_len;

   for (int i = 0; i < s_len; i++) {
      if (s[i] == ' ')
         continue;
      if (s[i] == '"') { ///string
         int str_i = i + 1;
         for (;; str_i++) {
            //printf("%c", s[str_i]);
            if (!(str_i < s_len))
               scm_error(PARSE_UNTERM_STR);
            if (s[str_i] == '\\')
               continue;
            if (s[str_i] == '"') {
               str_i++;
               break;
            }
         }
         curr_lex_len = str_i - i;
         curr_lex = malloc(curr_lex_len);
         strncpy(curr_lex, s + i, curr_lex_len);
         i = str_i;//-1;
      }
      else if (s[i] == '(' || s[i] == ')' || s[i] == '[' || s[i] == ']') { ///parens
         curr_lex = malloc(1);
         curr_lex[0] = s[i];
         curr_lex_len = 1;
      }
      else { ///text
         //TODO: check size here too
         int lex_i = i+1;
         char c;
         for (;; lex_i++) {
            c = s[lex_i];
            if (c == ' ' || c == '"' || c == '(' || c== ')' || c == '[' || c == ']')
               break;
            if (!(lex_i < s_len)) //(lex_i + 1 >= s_len)
               scm_error(PARSE_UNTERM_STR);
         }
         curr_lex_len = lex_i - i;
         curr_lex = malloc(curr_lex_len);
         //printf("\tcurr_lex: %i", curr_lex_len);
         strncpy(curr_lex, s + i, curr_lex_len);
         i = lex_i-1;
      }
      array_add(arr, curr_lex, curr_lex_len);
      free(curr_lex);
   }

   /*printf("\nnow stuff:");
   for (int i = 0; i < arr->num; i++) {
      printf("\nlexeme %i: ^", i);
      for (int j = 0; j < arr->size_each[i]; j++)
         printf("%c", arr->arr[i][j]);
      printf("^");
   }*/
   return arr;
}

struct Sexps;

union Elem {
   Sexps* rec;
   string* atom;
};

struct Sexps {
   vector<Elem> elems;

   void addStr(const string& s) {
      Elem e;
      e.atom = new string;
      *(e.atom) = s;
      elems.push_back(e);
   }
   void addSexps(Sexps *other) {
      Elem e;
      e.rec = other;
      elems.push_back(e);
   }
};



Sexps make_ast(vector<string> lexes) {
   Sexps sexps;

   for (int i = 0; i < lexes.size(); i++) {
      if (lexes[i] == "(") {
         int deep = 1;
         int j;
         for (j = i; deep != 0 && j < lexes.size(); j++) {
            if (lexes[j] == "(")
               deep++;
            if (lexes[j] == ")")
               deep--;
         }
         vector<string> sub_sexps_str_vec;
         for (int z = i; z < j; z++) {
            sub_sexps_str_vec.push_back(lexes[z]);
         }
         Sexps *sub_sexps = &make_ast(sub_sexps_str_vec);
         sexps.addSexps(sub_sexps);
      }
      else {
         sexps.addStr(lexes[i]);
      }
   }

}

int main() {
   const char* s = "(hello (+ fdsf \"   \" sdf) ewr)";

   array_t* lexes_c = lex(s);

   vector<string> lexes;

   array_for_each((*lexes_c), i) {
      //cout << lexes_c->arr[i];
      lexes.push_back(string(lexes_c->arr[i]));
   }

   Sexps sexps = make_ast(lexes);
}

/*int main1() {
   assert(arr_test());

   printf("\n\nHello world!\n");
   return 0;
}*/


