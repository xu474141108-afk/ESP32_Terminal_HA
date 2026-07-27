//  void ha_ws_subscribe_entities(int msg_id, const char **entity_ids, int count) {
//     cJSON *root = cJSON_CreateObject();
//     cJSON_AddNumberToObject(root, "id", msg_id);
//     cJSON_AddStringToObject(root, "type", "subscribe_entities");

//     cJSON *array = cJSON_CreateArray();
//     for (int i = 0; i < count; i++) {
//         cJSON_AddItemToArray(array, cJSON_CreateString(entity_ids[i]));
//     }
//     cJSON_AddItemToObject(root, "entity_ids", array);

//     char *out = cJSON_PrintUnformatted(root);
//     free(out);
//     cJSON_Delete(root);
// }