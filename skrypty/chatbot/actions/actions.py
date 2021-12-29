# This files contains your custom actions which can be used to run
# custom Python code.
#
# See this guide on how to implement these action:
# https://rasa.com/docs/rasa/custom-actions

from typing import Any, Text, Dict, List
from rasa_sdk import Action, Tracker
from rasa_sdk.executor import CollectingDispatcher
import json
import datetime
import os
import spacy
from nltk.stem.porter import *
from rasa_sdk.events import SlotSet

class ActionOpeningHours(Action):

    def name(self) -> Text:
        return "action_opening_hours"

    def run(self, dispatcher: CollectingDispatcher,
            tracker: Tracker,
            domain: Dict[Text, Any]) -> List[Dict[Text, Any]]:            

        f = open(os.path.join('data', 'opening_hours.json'))
        data = json.load(f)
        f.close()

        day_to_check = next(tracker.get_latest_entity_values("day_is_opened"), None)
        most_similar_idx = 0
        highest_similarity = 0
        nlp = spacy.load('en')
        days_list = list(data['items'].keys())
        how_similars = []
        if day_to_check is not None:
            day_to_check = day_to_check.capitalize()
            for i in range(len(days_list)):
                how_similar = nlp(days_list[i]).similarity(nlp(day_to_check))
                how_similars.append(how_similar)
                if how_similar > highest_similarity:
                    highest_similarity = how_similar
                    most_similar_idx = i
        day_to_check = days_list[most_similar_idx]
        time_to_check = next(tracker.get_latest_entity_values("time_is_opened"), None)
        opening_hours = data["items"][day_to_check]
        if time_to_check is not None:
            if ':' not in time_to_check:
                time_to_check += ":00"
            time_to_check = datetime.datetime.strptime(time_to_check, "%H:%M")

        if day_to_check is not None and time_to_check is not None:
            opening_hour = datetime.datetime.strptime(str(opening_hours["open"]) + ":00", "%H:%M")
            closing_hour = datetime.datetime.strptime(str(opening_hours["close"]) + ":00", "%H:%M")
            time_to_check_minute = time_to_check.minute
            if time_to_check_minute < 10:
                time_to_check_minute = "0" + str(time_to_check_minute)
            if (time_to_check >= opening_hour and time_to_check <= closing_hour):
                dispatcher.utter_message(text=f"Yes, restaurant is opened on {day_to_check} at {time_to_check.hour}:{time_to_check_minute}.")
            else:
                dispatcher.utter_message(text=f"No, restaurant is closed on {day_to_check} at {time_to_check.hour}:{time_to_check_minute}.")

        elif day_to_check is not None:
            if opening_hours["open"] == 0 and opening_hours["close"] == 0:
                dispatcher.utter_message(text=f'No, restaurant is closed on {day_to_check}')
            else:
                dispatcher.utter_message(text=f'Yes, restaurant on {day_to_check} is opened from {opening_hours["open"]} to {opening_hours["close"]}.')
        elif time_to_check is not None:
            days_opened = []
            for day in data["items"]:
                opening_hours = data["items"][day]
                opening_hour = datetime.datetime.strptime(str(opening_hours["open"]) + ":00", "%H:%M")
                closing_hour = datetime.datetime.strptime(str(opening_hours["close"]) + ":00", "%H:%M")
                if time_to_check >= opening_hour and time_to_check <= closing_hour:
                    days_opened.append(day)
            time_to_check_minute = time_to_check.minute
            if time_to_check_minute < 10:
                time_to_check_minute = "0" + str(time_to_check_minute)
            dispatcher.utter_message(text=f"At {time_to_check.hour}:{time_to_check_minute} the restaurant is opened on {', '.join(days_opened)}.")

        return []

class ActionMenuItems(Action):

    def name(self) -> Text:
        return "action_menu_items"

    def run(self, dispatcher: CollectingDispatcher,
            tracker: Tracker,
            domain: Dict[Text, Any]) -> List[Dict[Text, Any]]:            

        f = open(os.path.join('data', 'menu.json'))
        data = json.load(f)
        f.close()
        menu_items = [f"{item['name']} for {item['price']}$, prepared in {int(item['preparation_time'] * 60)} minutes." for item in data["items"]]
        dispatcher.utter_message(text="Our menu:\n" + '\n'.join(menu_items) + \
                                '\n\nWhen placing an order, please:\n1. Provide the amount of each order item\n2. Put any additional request or comment in round brackets: (comment)\n')

        return []

class ActionPlaceOrder(Action):

    def name(self) -> Text:
        return "action_place_order"

    def run(self, dispatcher: CollectingDispatcher,
            tracker: Tracker,
            domain: Dict[Text, Any]) -> List[Dict[Text, Any]]:            

        f = open(os.path.join('data', 'menu.json'))
        menu_items = json.load(f)['items']
        f.close()
        order = tracker.latest_message['entities']
        nlp = spacy.load('en')
        stemmer = PorterStemmer()
        similarity_treshold = 0.8
        for item_order in filter(lambda x: x['entity'] == 'menu_item_order', order):
            order_item_in_menu = False
            item_order_value = nlp(stemmer.stem(item_order['value']))
            for menu_item in menu_items:
                if nlp(menu_item['name'].lower()).similarity(item_order_value) >= similarity_treshold:
                    order_item_in_menu = True
                    break
            if order_item_in_menu is False:
                dispatcher.utter_message(text=f"No item {item_order['value']} found inside the menu. Either it's not there or there's a typo in your order. Please specify your order again.")
                return []
        entities_to_save = tracker.latest_message['entities']
        counts = []
        menu_item_orders = []
        comments = []
        last_entity_type = None
        for entity_to_save in entities_to_save:
            if last_entity_type == 'menu_item_order' and entity_to_save['entity'] == 'count':
                comments.append('-')
            if entity_to_save['entity'] == 'count':
                counts.append(entity_to_save['value'])
            elif entity_to_save['entity'] == 'menu_item_order':
                menu_item_orders.append(nlp(stemmer.stem(entity_to_save['value'])))
            elif entity_to_save['entity'] == 'comment':
                comments.append(entity_to_save['value'].strip("()"))
            last_entity_type = entity_to_save['entity']
        if last_entity_type == 'menu_item_order':
            comments.append('-')
        dispatcher.utter_message(text=f"{counts, menu_item_orders, comments}")
        return [SlotSet("count", counts),
                SlotSet("menu_item_order", menu_item_orders),
                SlotSet("comment", comments)]

class ActionShowOrder(Action):

    def name(self) -> Text:
        return "action_show_order"

    def run(self, dispatcher: CollectingDispatcher,
            tracker: Tracker,
            domain: Dict[Text, Any]) -> List[Dict[Text, Any]]:            

        counts = tracker.get_slot('count')
        menu_item_orders = tracker.get_slot('menu_item_order')
        comments = tracker.get_slot('comment')

        sentence = 'Your order:\n'
        for menu_item in zip(counts, menu_item_orders, comments):
            sentence += f'Count: {menu_item[0]}, menu item: {menu_item[1]}, comment: {menu_item[2]} + \n'

        dispatcher.utter_message(text=sentence)
        return []