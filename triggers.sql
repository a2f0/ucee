--    
-- Trigger 1: duplicates insertions to t1--which may be changed, for example if there are partial fills--into t2, which will not be changed

DELIMITER |
	  
CREATE TRIGGER PersistOrdersToT2
BEFORE INSERT ON t1
FOR EACH ROW
    BEGIN
	   INSERT IGNORE INTO t2 VALUES(NEW.t1key, NEW.account, NEW.user, NEW.ordertype, NEW.timestamp, NEW.side, NEW.symbol, NEW.price, NEW.quantity);
    END; |
				    
DELIMITER ;

