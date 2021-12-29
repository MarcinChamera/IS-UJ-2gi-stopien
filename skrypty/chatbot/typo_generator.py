import sys
import os
import random
import string

filename = sys.argv[1]
f_r = open(os.path.join('intents_examples', filename), 'r')
num_lines = sum(1 for _ in f_r)
f_r.close()
f_r = open(os.path.join('intents_examples', filename), 'r')
f_w = open(os.path.join('intents_examples_with_typos', filename), 'a')
to_continue = [' ', '[', ']', ':', '(', ')']
to_continue.extend(map(lambda x: str(x), list(range(10))))
to_activate_skip = ['(', '{']
to_deactivate_skip = [')', '}']
punctuation_to_delete = ['.', '?']
p = 0.05
for line_idx, line in enumerate(f_r):
    if line_idx < num_lines / 2:
        f_w.write(line)
    else:
        skip = False
        sentence = line.split('- ', 1)[1]
        sentence_with_typo = sentence
        for idx in range(len(sentence))[:-1]:
            if sentence[idx] in to_continue:
                continue
            if sentence[idx] in to_activate_skip:
                skip = True
            elif sentence[idx] in to_deactivate_skip:
                skip = False
                continue
            if skip is False:
                new_character = sentence[idx]
                if sentence[idx] in punctuation_to_delete and random.random() < 0.5:
                    new_character = ' '
                elif sentence[idx] == '-':
                    new_character = random.choice(['-', ' '])
                    p *= p
                elif random.random() < p:
                    new_character = random.choice(string.ascii_lowercase)
                    p *= p
                else:
                    p = 0.05
                sentence_with_typo = sentence_with_typo[:idx] + new_character + sentence_with_typo[idx + 1:]
        f_w.write('- ' + sentence_with_typo)


f_r.close()
f_w.close()