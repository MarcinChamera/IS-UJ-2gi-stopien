import os
import json
import random
 
POSSIBLE_COUNT = 5
SENTENCES = 25
GROUPS = 5

def get_menu_item_entity(group):
    return '{"entity": "menu_item_order", "group": "' + group + '"}'

def get_comment_entity(group):
    return '{"entity": "comment", "group": "' + group + '"}'

def get_count_entity(group):
    return '{"entity": "count", "group": "' + group + '"} '

start = ["I want to order ", "I'd like to order ", "I would like to order ", "Order ", ""]

count = [f'[{str(i)}]' for i in range(1, POSSIBLE_COUNT)]

f_menu = open(os.path.join('data', 'menu.json'), 'r')
existing_menu_items = [item['name'] for item in json.load(f_menu)['items']]
existing_menu_items = ['[' + existing_menu_item + ']' for existing_menu_item in existing_menu_items]
not_existing_menu_items = ['[Sushi]', '[Kebab]']
menu_items = existing_menu_items + not_existing_menu_items
f_menu.close()

comments = ['without meat', 'with additional lettuce', 'with extra red cabbage', 'no mustard', 'with no ketchup', ' no pineapple please', 
            'without olives', 'extra onion please', "i'm allergic to soy", "i'm allergic to milk", 'please deliver asap']
comments = [f'[({comment})]' for comment in comments]

f_place_order = open(os.path.join('intents_examples', 'generated_place_order_examples.txt'), 'w')
f_place_order.write(f'# Generated with {os.path.basename(__file__)}\n\n')

for sentence_idx in range(SENTENCES):
    random.shuffle(start)
    sentence = random.choice(start)
    group_numbers = random.randint(1, GROUPS)
    menu_items_to_choose_from = menu_items.copy()
    for group_number in range(1, group_numbers + 1):
        group_number = str(group_number)
        random.shuffle(count)
        random_count = random.choice(count)
        plural = False
        if random_count != "":
            if random_count != "[1]":
                plural = True
            sentence = sentence + random_count + get_count_entity(group_number)
        random.shuffle(menu_items_to_choose_from)
        random_menu_item = random.choice(menu_items_to_choose_from)
        menu_items_to_choose_from.remove(random_menu_item)
        print(f'Zdanie {sentence_idx}: Wybralem {random_menu_item}, na liscie zostaly: {menu_items_to_choose_from}')
        if plural:
            random_menu_item = random_menu_item[:-1] + "s]"
        sentence = sentence + random_menu_item + get_menu_item_entity(group_number)
        if random.random() >= 0.5:
            random.shuffle(comments)
            sentence = sentence + ' ' + random.choice(comments) + get_comment_entity(group_number)
        if int(group_number) < group_numbers - 1:
            sentence += ', '
    f_place_order.write(f'- {sentence}\n')
    print(f'Na koniec zdania {sentence_idx} na liscie do wyboru zostaly: {menu_items_to_choose_from}')

f_place_order.close()