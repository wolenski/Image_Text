## extract data for training
import os


def extract_training_data(file_name, is_positive):

    items = ""
    url = ""

    # dict for url
    url_dict = {}
    # array for training data
    arr = []
    file_to_read = open(file_name, 'r')
    for line in file_to_read:
        if len(line) <= 0:
            break
        if line.find("url:") != -1:
        	# skip "url:http://"
            url = line.strip()[11:]    
            url_dict[url] = 1       
            continue
        if line.find("xpath:") != -1:
            continue
        items = line.strip()  
        if line.find("float_left:") != -1:
            arr.append(items)
            items = ""
        #print items
    file_to_read.close() 

    print len(arr)
    #output the urls into  file
    # url_fname = file_name + "_url"
    # file_to_write = open(url_fname,"w") 
    # url_keys = url_dict.keys()
    # for u in url_keys:
    #     file_to_write.write(str(u) + "\n")
    # file_to_write.close()  
    
    #output the features into  file
    out_fname =  file_name + "_train_data"
    out_fname =  "tree_models_data.trn"
    out_fname =  "tree_models_data.tst"
    # dictionary for {string : int} mapping
    data_dict = {}
    file_to_write = open(out_fname,"a")    
    for items in arr: 
        if len(items) < 10:
            print "Warning! array item error!"
            break
        data_dict.clear()
        #print items
        pairs = items.split(",")
        for pair in pairs:
            values = pair.split(":")
            if len(values) < 2:
                print "Warning! less than 2 items in one pair!"
                print line
                break
            data_dict[values[0]] = int(values[1])

        if data_dict["block_wx"] < data_dict["img_wx"]:
            data_dict["block_wx"] = data_dict["img_wx"]
        if data_dict["block_hx"] < data_dict["img_hx"]:
            data_dict["block_hx"] = data_dict["img_hx"]

        block_pos_x = data_dict["block_pos_x"]
        block_pos_y = data_dict["block_pos_y"]    
        block_wx = data_dict["block_wx"]
        block_hx = data_dict["block_hx"]
        img_pos_x = data_dict["img_pos_x"]
        img_pos_y = data_dict["img_pos_y"]
        img_wx = data_dict["img_wx"]
        img_hx = data_dict["img_hx"]        
        anchor_count = data_dict["anchor_count"]
        sub_text_size = data_dict["sub_text_size"]
        in_tag_a = data_dict["in_tag_a"]
        float_left = data_dict["float_left"]
        margin_left = img_pos_x - block_pos_x
        margin_top = img_pos_y - block_pos_y
        margin_right = block_wx - img_wx - margin_left
        margin_bottom = block_hx - img_hx - margin_top
        block_area = block_wx * block_hx
        img_area = img_wx * img_hx
        # margin area
        margin_left_area = margin_left * img_hx
        margin_top_area = margin_top * block_wx
        margin_right_area = margin_right * img_hx
        margin_bottom_area = margin_bottom * block_wx

        ##############################
        ### normalization
        ##############################
        fea_block_wx = float(block_wx) / 1024
        fea_block_hx = float(block_hx) / 1024
        fea_img_wx = float(img_wx) / 1024
        fea_img_hx = float(img_hx) / 1024
        fea_anchor_count = float(anchor_count) / (sub_text_size + 1)
        # regard one word as 14px * 14px
        fea_sub_text_size = float(sub_text_size) * 200 / (block_area - img_area - margin_left_area + 1)
        fea_in_tag_a = float(in_tag_a) 
        fea_float_left = float(float_left)
        ##############################
        ## generalize new feature
        ##############################
        fea_img_block_ratio_wx = fea_img_wx / fea_block_wx
        fea_img_block_ratio_hx = fea_img_hx / fea_block_hx
        fea_img_block_ratio_margin_left = float(margin_left) / block_wx
        fea_img_block_ratio_margin_top = float(margin_top) / block_hx
        fea_img_block_ratio_margin_right = float(margin_right) / block_wx
        fea_img_block_ratio_margin_bottom = float(margin_bottom) / block_hx
        fea_img_block_ratio_area = fea_img_block_ratio_wx * fea_img_block_ratio_hx
        fea_img_block_ratio_margin_left_area = float(margin_left_area) / block_area
        fea_img_block_ratio_margin_top_area = float(margin_top_area) / block_area 
        fea_img_block_ratio_margin_right_area = float(margin_right_area) / block_area
        fea_img_block_ratio_margin_bottom_area = float(margin_bottom_area) / block_area

        feature = []
        feature.append(fea_block_wx)
        feature.append(fea_block_hx)
        feature.append(fea_img_wx)
        feature.append(fea_img_hx)
        feature.append(fea_anchor_count)
        feature.append(fea_sub_text_size)
        feature.append(fea_in_tag_a)
        feature.append(fea_float_left)
        feature.append(fea_img_block_ratio_wx)
        feature.append(fea_img_block_ratio_hx)
        feature.append(fea_img_block_ratio_margin_left)
        feature.append(fea_img_block_ratio_margin_top)
        feature.append(fea_img_block_ratio_margin_right)
        feature.append(fea_img_block_ratio_margin_bottom)
        feature.append(fea_img_block_ratio_area)
        feature.append(fea_img_block_ratio_margin_left_area)
        feature.append(fea_img_block_ratio_margin_top_area)
        feature.append(fea_img_block_ratio_margin_right_area)
        feature.append(fea_img_block_ratio_margin_bottom_area)

        file_to_write.write(str(is_positive) + "\t")
        count = 0
        for f in feature:
            file_to_write.write(str(count) + ":" + str(f) + "\t")
            count += 1
        file_to_write.write("\n")            
    file_to_write.close()

                                                                
if __name__ == '__main__':
    extract_training_data("positive_test", 1)
    extract_training_data("negative_test", 0)